//
// Created by app on 18-12-22.
//
#ifndef PROGRAM_1202_COMMON_H
#define PROGRAM_1202_COMMON_H

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include<opencv2/opencv.hpp>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <sys/types.h>
#include<iostream>
#include<string>
#include <typeinfo>

using namespace std;
using namespace cv;

#define GAUSS_DIFF 1
#define INIT_FILE "20180706/init.txt"

//const string FILE_PATH = "20180706/";
//const string MODEL_PATH = FILE_PATH+"model/";

/*************************************
Function:    remove_files()
Description: 删除目录下所有文件
Parameters:  path:目录路径
Return:      bool
**************************************/
bool remove_files(const char* path){
    DIR* dir = opendir(path);//打开指定目录
    dirent* p = NULL;//定义遍历指针
    try{
        while((p = readdir(dir)) != NULL)//开始逐个遍历
        {
            //过滤掉目录中"."和".."隐藏文件
            if(p->d_name[0] != '.')//d_name是一个char数组，存放当前遍历到的文件名
            {
                string name = string(p->d_name);
                remove((path+name).c_str());
//                cout<<(path+name).c_str()<<"000"<<endl;
            }
        }
        closedir(dir);
    }catch (exception &e){
        cerr << "Caught: " << e.what( ) << endl;
        cerr << "Type: " << typeid( e ).name( ) << endl << endl;
        return false;
    }
    return true;
}

Mat gauss_diff(Mat gray){
    Mat gauss,diff,tmp;
    GaussianBlur(gray, gauss, cv::Size(3,3),0,0);
    diff = gray-gauss;
    tmp = gray+10*diff;
    return tmp;
}

//获取旋转后图像
vector<Mat> rotation(Mat img,Point center)
{
    vector<Mat> rotation_imgs;
    Mat map_matrix,res,dst;

//    namedWindow("rotation",0);
//    resizeWindow("rotation",width/2,height/2);

    for(int i=-6;i<=6;i+=2){ //7度
        //map_matrix=getRotationMatrix2D(center,i,1.0);  //旋转中心，角度，缩放比例
        map_matrix=getRotationMatrix2D(center, i, 1.0);  //旋转中心，角度，缩放比例
        warpAffine(img,dst,map_matrix,Size(img.cols,img.rows));
        res = dst.clone();
        rotation_imgs.push_back(res);

//        imshow("rotation",res(Rect(img.cols*(1.0/3)/2,img.rows*(1.0/3)/2,(2.0/3)*img.cols,(2.0/3)*img.rows)));
//        cvWaitKey(5);
    }


    return rotation_imgs;
}

//获取平移后图像
vector<Mat> move(vector<Mat> rotate_imgs,Rect rect,int flag){
    Mat res;
    vector<Mat> move_imgs;
    Rect match=rect;
    for(vector<Mat>::iterator it=rotate_imgs.begin();it!=rotate_imgs.end();it++){//7张旋转图
        for(int i=-6;i<=6;i+=3){//5张平移
            if(flag==0)//上下矩形
            {match.y=rect.y+i;}
            else
            {match.x=rect.x+i;}
            res = (*it)(match).clone();
            if(GAUSS_DIFF)
                res = gauss_diff(res); //高反差
            move_imgs.push_back(res);
        }
    }
    return move_imgs;
}



void lineTran(Mat src,Mat &dst, float k,int b)
{
    for(int i = 0;i <src.rows; i++)
    {
        uchar *srcData = src.ptr<uchar>(i);
        for(int j = 0;j <src.cols; j++)
        {
            dst.at<uchar>(i,j) =round(srcData[j] *k) + b;
            if(dst.at<uchar>(i,j)>255)
                dst.at<uchar>(i,j)=255;
        }
    }
}


Mat laplace_enhance(Mat gray)
{
    if (gray.empty())
    {
        cerr << "打开图片失败,请检查" << std::endl;
    }
//    imshow("原图像", rgb);
    Mat imageEnhance;
    typedef cv::Matx<double, 3, 3> kernel;
    kernel m(0, -1, 0, 0, 3, 0, 0, -1, 0);
//    Mat kernel = (Mat_<char>(3, 3) << 0, -1, 0, 0, 5, 0, 0, -1, 0);

    filter2D(gray, imageEnhance, CV_8UC1, m);
//    imshow("拉普拉斯算子图像增强效果", imageEnhance);
//    waitKey(0);
    return imageEnhance;
}

void init(Mat img, double button_radius){
    Point center = Point(660,533); //纽扣中心
    double pixs_mm = 18.43;//单位长度对应像素距离 = 18.53pixs/mm
    int left_x = 595;//4个铁钉外接矩形左边缘x
    int top_y  = 470;//4个铁钉外接矩形上边缘y
    int bottom_y = center.y+center.y-top_y;//4个铁钉外接矩形下边缘y
    int right_x = center.x+center.x-left_x;//4个铁钉外接矩形右边缘x
//    int R = button_radius*pixs_mm; //纽扣像素半径

    int *pix,*pix1,*pix2;
    pix=rectImage(img);//截取外接圆=>pixs_mm
    pixs_mm = ((pix[2]+pix[3])/4.0)/button_radius;

    pix1=rectImage(img);
    center = Point((pix1[0]+pix1[2]/2),(pix1[1]+pix1[3]/2));
    left_x = pix1[0];
    top_y = pix1[1];
    cout<<"pix1[0]:"<<pix1[0]<<endl;

    pix2=rectImage(img);
    center = Point((center.x+(pix2[0]+pix2[2]/2))/2,(center.y+(pix1[1]+pix1[3]/2))/2);
    left_x = (left_x+pix2[0])/2;
    top_y = (top_y+pix2[1])/2;
    bottom_y = center.y+center.y-top_y;
    right_x = center.x+center.x-left_x;
    cout<<"pix2[0]:"<<pix2[0]<<endl;


    fstream file;
    try{
        file.open(INIT_FILE, ios::out);
        file.clear();//先清空文件
    }catch (exception &e){
        cerr<<e.what()<<endl;
        cerr<<"初始化文件打开失败!"<<endl;
        exit(0);
    }
    file << "center:"<<center.x<<","<<center.y<<endl;
    file << "top_y:"<<top_y<< endl;
    file << "left_x:"<<left_x<< endl;
    file << "bottom_y:"<<bottom_y<< endl;
    file << "right_x:"<<right_x<< endl;
    file << "pixs_mm(像素比):"<<pixs_mm<< endl;
    file.close();
}

//获取文件夹下子文件夹编号
int get_subdir_name(string Dir){
    int num=0;
    DIR* dir = opendir(Dir.c_str());//打开指定目录
    dirent* p = NULL;//定义遍历指针
    while((p = readdir(dir)) != NULL)//开始逐个遍历
    {
        //过滤掉目录中"."和".."隐藏文件
        if(p->d_name[0] != '.')//d_name是一个char数组，存放当前遍历到的文件名
        {
            string name = string(p->d_name);
            num = atoi(name.c_str())>num ? atoi(name.c_str()) : num;
        }
    }
    closedir(dir);
    return num;
}

#endif //PROGRAM_1202_COMMON_H
