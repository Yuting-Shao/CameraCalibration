/*Yuting Shao
* shao.yut@northeastern.edu
* CS 5330 Spring 2023
* Project 4 - Calibration and Augmented Reality, Tasks 4, 5, 6
*/

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace std;

// function drawing a virtual object on the chessboard
void drawObject(cv::Mat &img, const vector<cv::Point2f> &projectedPoints, const cv::Scalar color) {
    int basePoints = projectedPoints.size() - 1;

    // draw base lines
    for (int i = 0; i < basePoints - 1; i++) {
        cv::line(img, projectedPoints[i], projectedPoints[i + 1], color, 2);
    }
    cv::line(img, projectedPoints[basePoints - 1], projectedPoints[0], color, 2);

    // draw lines connecting base points to the apex
    for (int i = 0; i < basePoints; i++) {
        cv::line(img, projectedPoints[i], projectedPoints[basePoints], color, 2);
    }
}

int main(int argc, char * argv[]) {
    // Task 4. Calculate Current Position of the Camera
    // read camera calibration parameters from a file
    cv::Mat cameraMatrix, distortionCoefficients;
    cv::FileStorage fs("intrinsicParams.xml", cv::FileStorage::READ);
    fs["cameraMatrix"] >> cameraMatrix;
    fs["distortionCoefficients"] >> distortionCoefficients;
    fs.release();

    // open the default camera
    cv::VideoCapture cap(0);

    // check if camera is opened
    if(!cap.isOpened()) {
        cerr << "Error: unable to open the default camera." << endl;
        exit(-1);
    }

    // define the chessboard pattern size and create the vector to store the corner points
    cv::Size patternSize(9, 6);
    vector<cv::Point2f> cornerSet;
    vector<cv::Vec3f> pointSet;

    // generate pointSet
    for(int i = 0; i < patternSize.height; i++) {
        for(int j = 0; j < patternSize.width; j++) {
            pointSet.push_back(cv::Vec3f(j, -i, 0));
        }
    }

    // process each frame, detect corners and calculate the board's pose
    cv::Mat frame, grayFrame;
    cv::Mat rvec, tvec;

    // define the 3D points for the 3D axes
    vector<cv::Point3f> axesPoints;
    axesPoints.push_back(cv::Point3f(0, 0, 0));
    axesPoints.push_back(cv::Point3f(3, 0, 0));
    axesPoints.push_back(cv::Point3f(0, 3, 0));
    axesPoints.push_back(cv::Point3f(0, 0, 3));

    // define the 3D points for the virtual obejects
    vector<cv::Point3f> objectPoints1;
    vector<cv::Point3f> objectPoints2;
    int numBasePoints = 8;
    float radius = 0.5f;
    float height = 2.0f;
    cv::Point3f apex1(3, -3, height);
    cv::Point3f apex2(6, -4, height);

    for (int i = 0; i < numBasePoints; i++) {
        float angle = (2 * M_PI / numBasePoints) * i;
        float x = 3 + radius * cos(angle);
        float y = -3 + radius * sin(angle);
        objectPoints1.push_back(cv::Point3f(x, y, 0));
        x = 6 + radius * cos(angle);
        y = -4 + radius * sin(angle);
        objectPoints2.push_back(cv::Point3f(x, y, 0));
    }
    objectPoints1.push_back(apex1);
    objectPoints2.push_back(apex2);


    while(true) {
        // capture a frame
        cap >> frame;

        // check if captured successfuly
        if(frame.empty()) {
            cerr << "Error: unable to capture a frame." << endl;
            exit(-2);
        }

        // convert to gray
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

        // detect corners
        bool found = cv::findChessboardCorners(grayFrame, patternSize, cornerSet);

        if(found) {
            // refine the detected corners positions to sub-pixel accuracy
            cv::cornerSubPix(grayFrame, cornerSet, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));

            // estimate the board's pose (rotation and translation)
            cv::solvePnP(pointSet, cornerSet, cameraMatrix, distortionCoefficients, rvec, tvec);

            // print the rotation and translation data
            cout << "Rotation vector: " << rvec << endl;
            cout << "Translation vector: " << tvec << endl;

            // Task 5. Project 3D axes at the origin of the image plane
            vector<cv::Point2f> projectedPoints;
            cv::projectPoints(axesPoints, rvec, tvec, cameraMatrix, distortionCoefficients, projectedPoints);
            
            int arrowSize = 9;

            // draw X axis (blue line and arrow)
            cv::arrowedLine(frame, projectedPoints[0], projectedPoints[1], cv::Scalar(255, 0, 0), 2, 8, 0, 0.2);

            // draw Y axis (green line and arrow)
            cv::arrowedLine(frame, projectedPoints[0], projectedPoints[2], cv::Scalar(0, 255, 0), 2, 8, 0, 0.2);

            // draw Z axis (red line and arrow)
            cv::arrowedLine(frame, projectedPoints[0], projectedPoints[3], cv::Scalar(0, 0, 255), 2, 8, 0, 0.2);

            // Task 6. Create Virtual Objects
            vector<cv::Point2f> projectedObjectPoints1, projectedObjectPoints2;
            cv::projectPoints(objectPoints1, rvec, tvec, cameraMatrix, distortionCoefficients, projectedObjectPoints1);
            cv::projectPoints(objectPoints2, rvec, tvec, cameraMatrix, distortionCoefficients, projectedObjectPoints2);
            // draw the projected objects on the image frame
            cv::Scalar orange(0, 165, 255);
            cv::Scalar darkGreen(0, 128, 0);
            drawObject(frame, projectedObjectPoints1, orange);
            drawObject(frame, projectedObjectPoints2, darkGreen);
        }

        cv::imshow("Frame with 3D Axes", frame);

        // exit on key 'q' press
        if(cv::waitKey(30) == 'q') break;
    }

    // release the camera and close window
    cap.release();
    cv::destroyAllWindows();

    return 0;
}