/*Yuting Shao
* shao.yut@northeastern.edu
* CS 5330 Spring 2023
* Project 4 - Calibration and Augmented Reality, Tasks 1, 2, 3
*/

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace std;

int main(int argc, char * argv[]) {
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
    vector<vector<cv::Point2f>> cornerList;
    vector<cv::Vec3f> pointSet;
    vector<vector<cv::Vec3f>> pointList;

    // generate pointSet
    for(int i = 0; i < patternSize.height; i++) {
        for(int j = 0; j < patternSize.width; j++) {
            pointSet.push_back(cv::Vec3f(j, -i, 0));
        }
    }

    // process each frame, detect corners and draw them
    cv::Mat frame, grayFrame, recentDetectedFrame;
    int cntCalibratedImage = 1;
    bool showInfoCorners = true;
    while(true) {
        // capture a frame
        cap >> frame;

        // check if captured successfuly
        if(frame.empty()) {
            cerr << "Error: unable to capture a frame." << endl;
            exit(-2);
        }

        // Task 1. Detect and Extract Chessboard Corners

        // convert to gray
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

        // detect corners
        bool found = cv::findChessboardCorners(grayFrame, patternSize, cornerSet);

        // draw the corners if found
        if(found) {
            // refine the detected corners positions to sub-pixel accuracy
            cv::cornerSubPix(grayFrame, cornerSet, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));

            // draw corners
            cv::drawChessboardCorners(frame, patternSize, cv::Mat(cornerSet), found);

            recentDetectedFrame = frame.clone();

            if(showInfoCorners){
                cout << "corners found: " << cornerSet.size() << endl;
                cout << "1st corner coordinates: (" << cornerSet[0].x << ", " << cornerSet[0].y << ")" << endl;
                // turn off the output
                showInfoCorners = false;
            }
        }

        cv::imshow("Frame", frame);

        // exit on key 'q' press
        if(cv::waitKey(30) == 'q') break;

        // Task 2. Select Calibration Images
        if(cv::waitKey(30) == 's') {
            // save the detected corners to the list
            cornerList.push_back(cornerSet);
            pointList.push_back(pointSet);

            // save the calibration image
            string fileName = "calibrationImage" + to_string(cntCalibratedImage) + ".png";
            cv::imwrite(fileName, recentDetectedFrame);
            cout << "number of calibrated frames: " << cntCalibratedImage << endl;
            cntCalibratedImage++;
        }

        // Task 3. Calibrate the Camera
        if(cv::waitKey(30) == 'c') {
            if(cornerList.size() >= 5){
                cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64FC1);
                cameraMatrix.at<double>(0, 2) = frame.cols / 2.0;
                cameraMatrix.at<double>(1, 2) = frame.rows / 2.0;

                cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64FC1);
                vector<cv::Mat> rvecs, tvecs;

                cout << "Initial Camera Matrix:" << endl << cameraMatrix << endl;
                cout << "Initial Distortion Coefficients:" << endl << distCoeffs << endl;

                double error = cv::calibrateCamera(pointList, cornerList, frame.size(), cameraMatrix, distCoeffs, rvecs, tvecs, cv::CALIB_FIX_ASPECT_RATIO);

                cout << "Calibrated Camera Matrix:" << endl << cameraMatrix << endl;
                cout << "Calibrated Distortion Coefficients:" << endl << distCoeffs << endl;
                cout << "Re-projection error:" << error << " pixels" <<  endl;

                // save intrinsic parameters to a file
                cv::FileStorage fs("intrinsicParams.xml", cv::FileStorage::WRITE);
                fs << "cameraMatrix" << cameraMatrix;
                fs << "distortionCoefficients" << distCoeffs;
                fs.release();

                // store rotations and translations
                ofstream outFile("rotationsTranslations.txt");
                for (size_t i = 0; i < rvecs.size(); i++) {
                    // convert rotation vector to rotation matrix
                    cv::Mat R;
                    cv::Rodrigues(rvecs[i], R);

                    // write the rotation matrix and translation vector to the file
                    outFile << "Rotation matrix for image " << i << ":\n" << R << "\n";
                    outFile << "Translation vector for image " << i << ":\n" << tvecs[i] << "\n\n";
                }

                outFile.close();

                cout << "Calibrate camera with " << cornerList.size() << " frames done." << endl;
            }
            else{
                cout << "At least 5 frames are needed to do a camera calibration. Currently " << cornerList.size() << " frames saved." << endl; 
            }
        }
    }

    // release the camera and close window
    cap.release();
    cv::destroyAllWindows();

    return 0;
}