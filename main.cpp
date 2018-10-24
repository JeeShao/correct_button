#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include<opencv2/opencv.hpp>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <sys/types.h>
#include<iostream>
#include<string>
#include<time.h>
#include <math.h>
#include "rect_image.h"
#include "match.h"
#include "serial.h"
#include "JHCap.h"
#include "bmpfile.h"

#define PI 3.1415926

using namespace std;
using namespace cv;

const string FILE_PATH = "20180706/";
const string MODEL_PATH = FILE_PATH+"model/";
const string ORG_PATH = FILE_PATH+"org_imgs/";
const string RECT_PATH = FILE_PATH+"org_rects/"; //模板图矩形框

TTY_INFO *ptty;  //串口
unsigned char DATA_MSG[] = {0xDE,0xA9,00,0xFF,0xFF};  //通知
unsigned char DATA_ANGLE[] = {0xDE,0xA1,01,00,0xFF,0xFF}; //旋转角度
unsigned char DATA_WIDTH[] = {0xDE,0xFC,00,0xFF,0xFF}; //纽扣宽度
unsigned char DATA_REC[8] = {}; //接受串口信息

int width=1280/*376*/, height=1024/*240*/, len=0;
IplImage *image = cvCreateImage(cvSize(width, height), 8, 3);
Point start1 = Point(640,0);
Point end1 = Point(640,1024);
Point start2 = Point(0,647);
Point end2 = Point(1280,647);


int generate_temp(){
    int num = 0,cur_no=0,sum=4;  //num当前最大模板编号  sum采集模板图片数（4） cur_no当前采集模板编号
    char key;
    double bot_width=0;
    //删除原模板文件
    DIR* dir = opendir(MODEL_PATH.c_str());//打开指定目录
    dirent* p = NULL;//定义遍历指针
    while((p = readdir(dir)) != NULL)//开始逐个遍历
    {
        //过滤掉目录中"."和".."隐藏文件
        if(p->d_name[0] != '.')//d_name是一个char数组，存放当前遍历到的文件名
        {
            string name = string(p->d_name);
//            cout<<(MODEL_PATH+name).c_str()<<endl;
            remove((MODEL_PATH+name).c_str());
        }
    }
    closedir(dir);

    //从摄像头读取4个角度模板原图.保存至20180706/
    Mat frame;

    namedWindow("frame",1);
//    DATA_MSG[2] = 03;  //完成
//    sendnTTY(ptty,DATA_MSG,5);//准备好
    while(0){
        key = waitKey(10);
        if(CameraQueryImage(0,(unsigned char*)image->imageData, &len,
                            CAMERA_IMAGE_BMP/* |CAMERA_IMAGE_TRIG*/)==API_OK)
        {
            frame = cvarrToMat(image);
            line(frame,start1,end1,(0,0,255));
            line(frame,start2,end2,(0,0,255));
            imshow("frame", frame);
        }
        if(key == 32){
            cout<<cur_no<<endl;
            imwrite((ORG_PATH+to_string(cur_no++)+".jpg").c_str(),frame);
        }
        if(key==27 || cur_no>=sum){
            return 0;
        }
//        if(recvnTTY(ptty,DATA_REC,4)==4)//拍照
//        {
//            printf("0x%02x",DATA_REC[1]);
//            if(DATA_REC[1]==0xFD)//拍照
//            {
//                if(sendnTTY(ptty,DATA_MSG,5)==5){ //完成
//                    cout<<cur_no<<endl;
//                    imwrite((ORG_PATH+to_string(cur_no++)+".jpg").c_str(),*image->imageData);//图片保存到本工程目录中
//                }else
//                    cout<<"send code error";
//            }else if(DATA_REC[1]==0xFE)  //退出模板
//                break;
//            memset(DATA_REC,0,8);
//        }
    }
    destroyWindow("frame");

//    while(recvnTTY(ptty,DATA_REC,5)!=5);
    if(DATA_REC[1]==0xFC){
        bot_width = DATA_REC[2];
    }
    memset(DATA_REC,0,8);

    /*
     * 计算区域的参数
     */
    double button_radius = 11.45; //纽扣半径 mm

    Point center = Point(710,594); //纽扣中心
    double section_angle = PI/3; //每一个截取区域的角度 60度
    double pixs_mm = 18.53;//单位长度对应像素距离 = 18.53pixs/mm
    int left_x = 647;//4个铁钉外接矩形左边缘x
    int top_y  = 529;//4个铁钉外接矩形上边缘y
    int R = button_radius*pixs_mm; //纽扣像素半径

    /*
     * 自动计算2块截取区域
     * 矩形顶点坐标可以适当往里面缩进一些，宽度和高度也可以适当减小，对宽高设置阈值判断 增加容错
     */
    CvRect rect1, rect2;
    //top area
    rect1.x = center.x-R*sin(section_angle/2);
    rect1.y = center.y-R*cos(section_angle/2);
    rect1.width = 2*R*sin(section_angle/2);
    rect1.height = (2.0/3)*(top_y-rect1.y);//区域高度设为区域顶部到铁钉上边缘的2/3
    //left area
    rect2.x = center.x-R*cos(section_angle/2);
    rect2.y = center.y-R*sin(section_angle/2);
    rect2.width = (2.0/3)*(left_x-rect2.x);//区域宽度设为区域左部到铁钉左边缘的2/3
    rect2.height = 2*R*sin(section_angle/2);

//    //top area
//    const int match_X = center.x-R*sin(section_angle/2);
//    const int match_Y = center.y-R*cos(section_angle/2);
//    const int match_Width = 2*R*sin(section_angle/2);
//    const int match_Height = (2/3)*(top_y-match_Y);//区域高度设为区域顶部到铁钉上边缘的2/3
//    //left area
//    const int match_X2 = center.x-R*cos(section_angle/2);
//    const int match_Y2 = center.y-R*sin(section_angle/2);
//    const int match_Width2 = (2/3)*(left_x-match_Y2);//区域宽度设为区域左部到铁钉左边缘的2/3
//    const int match_Height2 = 2*R*sin(section_angle/2);
    /*
     * 在原图上画出截取区域矩形 并显示保存
     */



    //先清空文件
    fstream file;
    file.open(MODEL_PATH+"/model.txt", ios::out);
    if (!file.is_open()){
        cout << "模板文件打开失败!" << endl;
        return 0;
    }
    file.clear();

    //待匹配区域为手选区域zhongjian部分的边缘缩进4 pixs
    file << "待匹配区域的参数1:" << rect1.x+4 << "," << rect1.y+4 << "," << rect1.width-8 << "," << rect1.height-8 << endl;
    file << "待匹配区域的参数2:" << rect2.x+4 << "," << rect2.y+4 << "," << rect2.width-8 << "," << rect2.height-8 << endl;
    file.close();
    //获取原图像
    unsigned int index = 0;
    unsigned int total = 4;
    char filename[100];
    //放模板匹配区域的图像
    vector<IplImage*> modelImages;
    //原图像
    IplImage* srcImage; /* = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);*/
    //读取文件夹中的图像
    for (index; index < total; index++){
        sprintf(filename, (ORG_PATH+to_string(index)+".jpg").c_str());
        srcImage = cvLoadImage(filename, 1);

        if (srcImage != NULL){
            //灰度处理
            Mat blurModelDstMat = pertImage(srcImage);
            IplImage temp_1 = (IplImage)blurModelDstMat;

            IplImage* blurImage = &temp_1 ;
            IplImage* matchImage ;
            //存入容器中
            matchImage = getRectImage(blurImage, rect1.x, rect1.y, rect1.width, rect1.height);
            modelImages.push_back(matchImage);
            matchImage = getRectImage(blurImage, rect2.x, rect2.y, rect2.width, rect2.height);
            modelImages.push_back(matchImage);

            rectangle(blurModelDstMat,rect1,Scalar(0,0,255),2,1,0);  //在模板中画矩形框 并保存
            rectangle(blurModelDstMat,rect2,Scalar(0,0,255),2,1,0);
            imwrite((RECT_PATH+to_string(index)+".jpg").c_str(), blurModelDstMat);
        }
        else{
            cout << "模板图像不足！" << endl;
            break;
        }
    }
    //取出匹配区域进行二值化并保存，其实可以不用存入容器中的，直接在上面二值化保存就行
    unsigned int vect_index = 0;
    for (vector<IplImage*>::iterator it = modelImages.begin(); it != modelImages.end(); it+=2)
    {
        Mat src = cvarrToMat(*it);
        sprintf(filename, (MODEL_PATH+"/"+to_string(vect_index)+'_'+'0'+".jpg").c_str());
        cout<<filename<<endl;
        imwrite(filename, src);

        src = cvarrToMat(*(it+1));
        sprintf(filename, (MODEL_PATH+"/"+to_string(vect_index)+'_'+'1'+".jpg").c_str());
        cout<<filename<<endl;
        imwrite(filename, src);

        vect_index = vect_index + 1;
    }

    cout <<"模板生成成功！" << endl;

    return 0;
}

int match(){
    char key;
    Mat frame;
    string match_num;

    DATA_MSG[2] = 03;  //完成
//    sendnTTY(ptty,DATA_MSG,5);//准备好
	//先读入模板model.txt
	ifstream fileinput;
    try {
        fileinput.open(MODEL_PATH+"/model.txt");
    }catch ( exception &e){
        cerr << "Caught: " << e.what( ) << endl;
        cerr << "Type: " << typeid( e ).name( ) << endl << endl;
    }

    if (!fileinput.is_open())
    {
        cout<<"模板不存在或打开模板文件失败！"<<endl;
        return 0;
    }
	string data[2];
	fileinput >> data[0];
	fileinput >> data[1];
	fileinput.close();

	//待匹配区域的坐标
	CvRect matchRect, matchRect2;
	string matchRectValue = exchange(data[0], ":")[1];
	vector<string> matchs = exchange(matchRectValue, ",");
	matchRect.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
	matchRect.y = atoi(matchs[1].c_str());
	matchRect.width = atoi(matchs[2].c_str());
	matchRect.height = atoi(matchs[3].c_str());

    cout<<matchRect.x<<" "<<matchRect.y<<" "<<matchRect.width<<" "<<matchRect.height<<endl;

    matchRectValue = exchange(data[1], ":")[1];
    matchs = exchange(matchRectValue, ",");
    matchRect2.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
    matchRect2.y = atoi(matchs[1].c_str());
    matchRect2.width = atoi(matchs[2].c_str());
    matchRect2.height = atoi(matchs[3].c_str());

    cout<<matchRect2.x<<" "<<matchRect2.y<<" "<<matchRect2.width<<" "<<matchRect2.height<<endl;


    //读入图片到容器中
	Mat modelMat;
	unsigned int index = 0;
	unsigned int total = 4;
	char filename[100];
	vector<Mat> modelMats;
	IplImage* matchImage = cvCreateImage(cvSize(matchRect.width, matchRect.height), IPL_DEPTH_8U, 1);
	for (index; index < total; index++){
        for(int no=0; no<2; no++){
            sprintf(filename, (MODEL_PATH+"/"+to_string(index)+'_'+to_string(no)+".jpg").c_str());
            matchImage = cvLoadImage(filename, 0);//读取灰度
            if (matchImage != NULL){
                Mat modelMat = cvarrToMat(matchImage);
                modelMats.push_back(modelMat);
            }
            else{
                cout<<"模板图像不足！" << endl;
                break;
            }
        }
	}

    IplImage* matchRectImage, *matchRectImage2;
    IplImage matchBlurImage;
    Mat blurMatchDstMat,dstMatchRect,dstMatchRect2/*,frame*/;
    int direction = 1;//旋转方向 0-左  1-右
    int nbyte = 0;
    int test_no = 0;//测试图编号

    float value = 0,value2=0;
    double max = 0;
    int angle = 0;
    int iter = 0;
    cv::Point matchLoc;
    //逐一匹配
//            double Time = (double)cvGetTickCount();
    clock_t start,finish;
    double total_time;

    frame = imread(("pp/test/"+to_string(test_no)+".jpg").c_str(), 1);
    while(1){
        direction=1;
        key = waitKey(10);
        imshow("frame", frame);

//        if(nbyte==4 && DATA_REC[1]==0xFD)  //拍照
        if(key == 32 && test_no<8)
        {
            cout<<test_no<<endl;
            frame = imread(("pp/test/"+to_string(test_no++)+".jpg").c_str(), 1);
            rectangle(frame,matchRect,Scalar(0,0,255),2,1,0);
            rectangle(frame,matchRect2,Scalar(0,0,255),2,1,0);
            imshow("frame", frame);
            blurMatchDstMat = pertImage0(frame);

            matchBlurImage = (IplImage)blurMatchDstMat;

            //截匹配区域
            matchRectImage = getRectImage(&matchBlurImage, matchRect.x, matchRect.y, matchRect.width, matchRect.height);
            matchRectImage2 = getRectImage(&matchBlurImage, matchRect2.x, matchRect2.y, matchRect2.width, matchRect2.height);
            dstMatchRect = cvarrToMat(matchRectImage);
            dstMatchRect2 = cvarrToMat(matchRectImage2);

            value = 0,value2=0;
            max = 0;
            angle = 0;
            iter = 0;
//            cv::Point matchLoc;
            //逐一匹配
//            double Time = (double)cvGetTickCount();

            start = clock();

            for (vector <Mat>::iterator it = modelMats.begin(); it != modelMats.end(); it+=2){
//                cvtColor(*it, *it, CV_BGR2GRAY);
//                cvtColor(*(it+1), *(it+1), CV_BGR2GRAY);

                value = temp_match(*it, dstMatchRect, matchLoc, cv::TM_CCORR_NORMED);
                value2 = temp_match(*(it+1), dstMatchRect2, matchLoc, cv::TM_CCORR_NORMED);
                value = (value+value2)/2;
                printf("this is %d angle,value = %f\n ", iter, value);
                if (max<value){
                    max = value;
                    angle = iter;
                }
                iter+=90;
            }
            finish = clock();
            total_time = (double)((finish-start)*1000/CLOCKS_PER_SEC);//ms
            printf("match run time = %gms\n", total_time);//毫秒
            if (angle==270){
                angle=90;
                direction = 0;
            }
            max>=0.90 ? printf("正确旋转角度 = %d ,方向(0-左 1-右)=%d\n\n", angle,direction) : printf("无法识别\n");
//           if(max>=0.94){
//               DATA_ANGLE[3] = direction;
//               DATA_ANGLE[4] = angle;
//               sendnTTY(ptty,DATA_ANGLE,6);
//           }else{//01图像无法使别
//               DATA_MSG[2] = 01;
//               sendnTTY(ptty,DATA_MSG,5);
//           }
        }
        else if(test_no>=8 || nbyte==4 && DATA_REC[1]==0xFE)//退出识别
        {
            return 0;
        }
    }
}

int main()
{
//    generate_temp();
    match();
    return 0;


    int key,count;
    ptty = readyTTY(0);
    int nbyte;
    unsigned char cc[16];

    if(ptty == NULL) {
        printf("readyTTY(0) error\n");
        return -1;
    }

    lockTTY(ptty);
    if(setTTYSpeed(ptty,9600)>0){  //设置波特率
        printf("setTTYSpeed() error\n");
        return -1;
    }
    if(setTTYParity(ptty,8,'N',1)>0){ //设置通讯格式
        printf("setTTYParity() error\n");
        return -1;
    }
    fcntl(ptty->fd,F_SETFL,FNDELAY);

    CameraGetCount(&count);
    printf("Camera count: %d\n", count);
//    if(count<1) return 0;
    CameraInit(0);
    CameraSetGain(0, 32);
    CameraSetExposure(0, 1000);

    CameraSetSnapMode(0, CAMERA_SNAP_CONTINUATION);
    //CameraSetSnapMode(0, CAMERA_SNAP_TRIGGER);

//    int width=1280/*376*/, height=1024/*240*/, len=0;
    CameraSetResolution(0, 2, &width, &height);
//    CameraGetResolution(0, 0, &width, &height);
    //CameraSetROI(0, 0, 0, width, height);
    CameraGetImageBufferSize(0, &len, CAMERA_IMAGE_BMP);

//    IplImage *image = cvCreateImage(cvSize(width, height), 8, 3);
    cvNamedWindow("rtImg");
    match();
    return 0;


    if(recvnTTY(ptty,DATA_REC,4)==4 && DATA_REC[1]==0xFB){ //开启实时图像
        memset(DATA_REC,0,8);
        while(!(recvnTTY(ptty,DATA_REC,4)==4 && DATA_REC[1]==0xF9)){
            memset(DATA_REC,0,8);
            if(CameraQueryImage(0,(unsigned char*)image->imageData, &len, CAMERA_IMAGE_BMP/* |CAMERA_IMAGE_TRIG*/)==API_OK)
            {
                cvShowImage("rtImg", image);
                cvWaitKey(20);
            }
        }
        memset(DATA_REC,0,8);
    }

    while(1)
    {
//        nbyte = recvnTTY(ptty,DATA_REC,4);
//        printf("0x%02x\n",DATA_REC[0]);
//
//        if(nbyte!=-1){
//            cout<<nbyte<<endl;
//
//            printf("0x%02x\n",DATA_REC[0]);
//        }


        if(recvnTTY(ptty,DATA_REC,4)==4){
            cout<<DATA_REC[1]<<endl;
            switch (DATA_REC[1]){
                case 0xFF: generate_temp();break;
                case 0xFA: match();break;
//                case 'q':capture.release(); return 1;
//                default:capture.release();return 0;
            }
        }
        memset(DATA_REC,0,8);
        sleep(1);
    }
}

