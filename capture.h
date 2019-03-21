//
// Created by app on 19-1-16.
//

#ifndef PROGRAM_1202_CAPTURE_H
#define PROGRAM_1202_CAPTURE_H

#include<opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
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

    Mat frame;
    Mat getShowImg();
    Mat getImg1();
    void open();
    void close();
    bool init(int exporsure,int gain);
    void setExposure(int value);
    void setGain(int value);
    Mat getNextFrame();
    int read();


private:
    std::queue<Mat> Q;
    int device;
    Mat showImg;
    Mat currentFrame;
    VideoCapture capture;

};


#endif //PROGRAM_1202_CAPTURE_H
