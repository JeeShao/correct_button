#ifndef PROGRAM_1202_CAMERA_H
#define PROGRAM_1202_CAMERA_H

#include "JHCap.h"
#include "bmpfile.h"
#include<opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

using namespace cv;
class Camera{
public:
    Camera(int width,int height);
    ~Camera();
    Mat frame;
    bool init(int exporsure,int gain);
    void close();
    int read();
    Mat getImg();
    void setExposure(int value);
    void setGain(int value);


private:
    int width;
    int height;
    int len;
    IplImage *image;
};


#endif //PROGRAM_1202_CAMERA_H
