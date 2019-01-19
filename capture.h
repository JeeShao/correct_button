//
// Created by app on 19-1-16.
//

#ifndef PROGRAM_1202_CAPTURE_H
#define PROGRAM_1202_CAPTURE_H

#include<opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <queue>
//#include<opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc.hpp>
//#include<opencv2/opencv.hpp>
using namespace cv;
class Capture {
public:
    Capture(int device);
    ~Capture();

    Mat read();
    Mat getImg();
    void open();
    void close();

private:
    std::queue<Mat> Q;
    int device;
    Mat frame;
    VideoCapture capture;
};


#endif //PROGRAM_1202_CAPTURE_H
