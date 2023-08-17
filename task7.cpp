/*Yuting Shao
* shao.yut@northeastern.edu
* CS 5330 Spring 2023
* Project 4 - Calibration and Augmented Reality, Task 7
*/

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>

using namespace std;

int main(int argc, char * argv[]) {
    // open the default camera
    cv::VideoCapture cap(0);

    // check if camera is opened
    if(!cap.isOpened()) {
        cerr << "Error: unable to open the default camera." << endl;
        exit(-1);
    }

    cv::Mat frame, grayFrame;
    
    while(true) {
        // capture a frame
        cap >> frame;

        // check if captured successfuly
        if(frame.empty()) {
            cerr << "Error: unable to capture a frame." << endl;
            exit(-2);
        }

        // Task 7: Detect Robust Features - SURF

        // convert to gray
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

        // initialize the SURF detector
        cv::Ptr<cv::xfeatures2d::SURF> detector = cv::xfeatures2d::SURF::create(400);

        // detect keypoints
        std::vector<cv::KeyPoint> keypoints;
        detector->detect(grayFrame, keypoints);

        // draw keypoints
        cv::drawKeypoints(frame, keypoints, frame, cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT);

        cv::imshow("Frame with SURF feature points", frame);

        // exit on key 'q' press
        if(cv::waitKey(30) == 'q') break;
    }

    // release the camera and close window
    cap.release();
    cv::destroyAllWindows();

    return 0;
}