#ifndef PROGRAM_MATCH_H
#define PROGRAM_MATCH_H
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include<opencv2/opencv.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <fstream>
#include<iostream>
#include<string>
#include<vector>
#include <typeinfo>

//#include <windows.h>
#endif //PROGRAM_MATCH_H
using namespace std;
using namespace  cv;

Mat pertImage0(IplImage* srcImage);
vector<string> exchange(string str, char* c);
IplImage* getRectImage(IplImage *srcImage, int x, int y, int width, int height);
double temp_match(cv::Mat image, cv::Mat tepl, cv::Point &point, int method);
double GetSim(const Mat& src1, const Mat& src2);
int match();