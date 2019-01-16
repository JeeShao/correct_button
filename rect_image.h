#ifndef PROGRAM_RECT_IMAGE_H
#define PROGRAM_RECT_IMAGE_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#endif //PROGRAM_RECT_IMAGE_H


using  namespace cv;

Mat pertImage(IplImage* srcImage);
void on_mouse(int event, int x, int y, int flags, void* ustc);
int* rectImage(Mat srcImage); //手动截图