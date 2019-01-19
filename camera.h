#ifndef PROGRAM_1202_CAMERA_H
#define PROGRAM_1202_CAMERA_H

#include "JHCap.h"
#include "bmpfile.h"
#include<opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

using namespace cv;
class Camera{
public:
    Camera(int width,int height);
    ~Camera();

    Mat read();
    Mat getImg();
//    void close();
    bool init();

private:
    Mat frame;
    int width;
    int height;
    int len;
    IplImage *image;
};


#endif //PROGRAM_1202_CAMERA_H
