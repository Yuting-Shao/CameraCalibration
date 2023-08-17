/*Yuting Shao
* shao.yut@northeastern.edu
* CS 5330 Spring 2023
* Project 4 - Calibration and Augmented Reality, Extension
* Get your AR system working with a target other than the 
* checkerboard, like a photo, painting, or object of your 
* choice that is not a checkerboard. Make it something where 
* it is easy to find 3-4 points on the object and determine 
* which point is which. Place an AR object somewhere in the 
* world relative to the object.
*/

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>


using namespace std;

int main(int argc, char * argv[]) {
    // load the reference image of the tennis racket
    cv::Mat refImage = cv::imread("tennisRacket.png", cv::IMREAD_GRAYSCALE);
    if (refImage.empty()) {
        cerr << "Error: unable to load the reference image." << endl;
        exit(-1);
    }

    // detect features and compute descriptors using ORB
    cv::Ptr<cv::Feature2D> orb = cv::ORB::create();
    vector<cv::KeyPoint> refKeypoints;
    cv::Mat refDescriptors;
    orb->detectAndCompute(refImage, cv::noArray(), refKeypoints, refDescriptors);

    // initialize the matcher (BruteForce-Hamming)
    cv::BFMatcher matcher(cv::NORM_HAMMING);

    // camera setup
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error: unable to open the camera." << endl;
        exit(-2);
    }

    cv::Mat frame, grayFrame;
    while(true) {
        // capture a frame
        cap >> frame;
        if (frame.empty()) {
            cerr << "Error: unable to capture a frame." << endl;
            exit(-3);
        }

        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

        vector<cv::KeyPoint> frameKeypoints;
        cv::Mat frameDescriptors;
        orb->detectAndCompute(grayFrame, cv::noArray(), frameKeypoints, frameDescriptors);

        vector<cv::DMatch> matches;
        matcher.match(refDescriptors, frameDescriptors, matches);

        // filter matches using a threshold - 50
        vector<cv::DMatch> goodMatches;
        for (const auto &match : matches) {
            if (match.distance < 50) {
                goodMatches.push_back(match);
            }
        }

        // if enough good matches are found, estimate the homography
        if(goodMatches.size() > 10) {
            vector<cv::Point2f> refPoints, framePoints;
            for(const auto &match : goodMatches) {
                refPoints.push_back(refKeypoints[match.queryIdx].pt);
                framePoints.push_back(frameKeypoints[match.trainIdx].pt);
            }

            cv::Mat homography = cv::findHomography(refPoints, framePoints, cv::RANSAC);

            // define the 3D points of the virtual tennis ball (in the racket's coordinate system)
            vector<cv::Point3f> virtualPoints;
            virtualPoints.emplace_back(0, 0, 0);
            virtualPoints.emplace_back(5, 0, 0);
            virtualPoints.emplace_back(0, 5, 0);
            virtualPoints.emplace_back(5, 5, 0);

            cv::Mat cameraMatrix, distortionCoefficients;
            cv::FileStorage fs("intrinsicParams.xml", cv::FileStorage::READ);
            fs["cameraMatrix"] >> cameraMatrix;
            fs["distortionCoefficients"] >> distortionCoefficients;
            fs.release();

            vector<cv::Point2f> projectedPoints;
            cv::perspectiveTransform(virtualPoints, projectedPoints, homography);
            // render the virtual tennis balls on the current frame
            for (const auto &point : projectedPoints) {
                cv::circle(frame, point, 5, cv::Scalar(0, 255, 0), -1);

                cv::Point2f highlightPoint(point.x - 2, point.y - 2);
                cv::circle(frame, highlightPoint, 2, cv::Scalar(255, 255, 255), -1);
            }
        }
        // show the augmented reality video
        cv::imshow("Augmented Reality", frame);

        if (cv::waitKey(30) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}