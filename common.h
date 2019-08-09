#ifndef PROGRAM_1202_COMMON_H
#define PROGRAM_1202_COMMON_H

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/opencv.hpp>
#include<fstream>
#include<dirent.h>
#include<sys/stat.h>
#include<vector>
#include<ctime>
#include<sys/types.h>
#include<iostream>
#include<fstream>
#include<string>
#include<typeinfo>
#include<map>
#include<linux/hdreg.h>
#include<sys/ioctl.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
using namespace std;
using namespace cv;

#define GAUSS_DIFF 0
#define PI 3.1415926
#define LAPLACE 0//拉普拉斯增强
#define BLUR 1 //模糊
#define THRESH 1 //自适应
#define RECT_PRECENT  3.15/4 // 3.0/4  //矩形占比2/3
#define X_MARGIN  90  //   100  //圆心和4个铁钉构成的矩形的左右边距
#define Y_MARGIN  90  //   90  //圆心和4个铁钉构成的矩形的上下边距
#define DEBUG 0 //调试log打印
//#define CAMERA 1 // 1--驱动相机  0--USB相机

extern const string FILE_PATH;
extern const string MODEL_PATH;
extern const string ORG_PATH;
extern const string RECT_PATH;
extern const string PARAMS_PATH;
extern const string INIT_FILE;
extern const string LOG_FILE;
extern const string LOGO_FILE;

extern double RADIUS;
extern int SYMMERY;
extern int HOLES;
extern int POSITION;
extern int EXPOSURE;
extern int GAIN;
extern string ANGLE_SHOW;
extern string STATUS_SHOW;


/*************************************
Function:    remove_files()
Description: 删除目录下所有文件
Parameters:  path:目录路径
Return:      bool
**************************************/
bool remove_files(const char* path);

//白平衡
Mat whiteBalance(Mat g_srcImage);

//图像预处理,灰度，滤波
Mat pertImage(Mat srcImage);

Mat gauss_diff(Mat gray);

//获取旋转后图像
vector<Mat> rotation(Mat img,Point center);

//获取平移后图像
vector<Mat> move1(vector<Mat> rotate_imgs,Rect rect,int flag);

//拉普拉斯增强
Mat laplace_enhance(Mat gray);

//读文件时对每一行的处理
vector<string> exchange(string str, char* c);

//模板匹配
double temp_match(cv::Mat image, cv::Mat tepl, cv::Point &point, int method);

string get_time( );

bool init_sys();

void writeLog(const char* logStr);

void save_params();

void read_params();

void getDate(char *dateNow);

int char2int(char input);

int int2char(char input);

void hexStrXor(char * HexStr1, char * HexStr2, char * HexStr);

bool systemCheck();

#endif
