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
#define GAUSS_DIFF 1//高反差开关
#define MOVE_PIXS 3

using namespace std;
using namespace cv;



const string FILE_PATH = "20180706/";
const string MODEL_PATH = FILE_PATH+"model/";
const string ORG_PATH = FILE_PATH+"org_imgs/";
const string TEST_PATH = "pp/";//测试图像文件路径
const string RECT_PATH = FILE_PATH+"org_rects/"; //模板图矩形框

TTY_INFO *ptty;  //串口
unsigned char DATA_MSG[] = {0xDE,0xA9,00,0xFF,0xFF};  //通知
unsigned char DATA_ANGLE[] = {0xDE,0xA1,01,00,0xFF,0xFF}; //旋转角度
unsigned char DATA_WIDTH[] = {0xDE,0xFC,00,0xFF,0xFF}; //纽扣宽度
unsigned char DATA_REC[8] = {}; //接受串口信息

int width=1280/*376*/, height=1024/*240*/, len=0;
IplImage *image = cvCreateImage(cvSize(width, height), 8, 3);
Point start1 = Point(width/2,0);
Point end1 = Point(width/2,height);
Point start2 = Point(0,height/2);
Point end2 = Point(width,height/2);

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
                remove((MODEL_PATH+name).c_str());
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

/*************************************
Function:    generate_temp()
Description: 模板生成
Parameters:  btn_r:纽扣半径
             symmetry:对称性
             position:字符位置
Return:      int
**************************************/
int generate_temp(string btn_r,int symmetry,int position){
    //删除原模板文件
    if(!remove_files(MODEL_PATH.c_str()))
        {cerr<<"删除圆模板文件失败"<<endl; exit(0);}
    //从摄像头读取4个角度模板原图.保存至20180706/
    Mat frame;
    /*
     * 计算区域的参数
     */
    double button_radius = atof(btn_r.c_str()); //纽扣半径 mm

    Point center = Point(710,594); //纽扣中心
    double section_angle = PI/3; //每一个截取区域的角度 60度
    double pixs_mm = 18.53;//单位长度对应像素距离 = 18.53pixs/mm
    int left_x = 647;//4个铁钉外接矩形左边缘x
    int top_y  = 529;//4个铁钉外接矩形上边缘y
    int bottom_y = center.y+center.y-top_y;//4个铁钉外接矩形下边缘y
    int right_x = center.x+center.x-left_x;//4个铁钉外接矩形右边缘x
    int R = button_radius*pixs_mm; //纽扣像素半径

//    Point center = Point(660,533); //纽扣中心
//    double section_angle = PI/3; //每一个截取区域的角度 60度
//    double pixs_mm = 18.43;//单位长度对应像素距离 = 18.53pixs/mm
//    int left_x = 595;//4个铁钉外接矩形左边缘x
//    int top_y  = 470;//4个铁钉外接矩形上边缘y
//    int bottom_y = center.y+center.y-top_y;//4个铁钉外接矩形下边缘y
//    int right_x = center.x+center.x-left_x;//4个铁钉外接矩形右边缘x
//    int R = button_radius*pixs_mm; //纽扣像素半径

    /* 自动计算4块截取区域
     * 矩形顶点坐标可以适当往里面缩进一些，宽度和高度也可以适当减小，对宽高设置阈值判断 增加容错
     */
    CvRect rect_top, rect_left,rect_bottom,rect_right;//上下左右区域
    //top area
    rect_top.x = center.x-R*sin(section_angle/2);
    rect_top.y = center.y-R*cos(section_angle/2);
    rect_top.width = 2*R*sin(section_angle/2);
    rect_top.height = (2.0/3)*(top_y-rect_top.y);//区域高度设为区域顶部到铁钉上边缘的2/3
    //left area
    rect_left.x = center.x-R*cos(section_angle/2);
    rect_left.y = center.y-R*sin(section_angle/2);
    rect_left.width = (2.0/3)*(left_x-rect_left.x);//区域宽度设为区域左部到铁钉左边缘的2/3
    rect_left.height = 2*R*sin(section_angle/2);
    //bottom area
    rect_bottom.x = center.x-R*sin(section_angle/2);
    rect_bottom.y = bottom_y+(1.0/3)*(center.y+R*cos(section_angle/2)-bottom_y);
    rect_bottom.width = 2*R*sin(section_angle/2);
    rect_bottom.height = 2*(rect_bottom.y-bottom_y);
    //right area
    rect_right.x = right_x+(1.0/3)*(center.x+R*cos(section_angle/2)-right_x);
    rect_right.y = center.y-R*sin(section_angle/2);
    rect_right.width = 2*(rect_right.x-right_x);//区域宽度设为区域左部到铁钉左边缘的2/3
    rect_right.height = 2*R*sin(section_angle/2);
    
    /*根据字符位置position确定区域顺序*/
    CvRect rects[4]={rect_top,rect_left,rect_bottom,rect_right};
    CvRect rect1, rect2,rect3,rect4;//字符方向顺序区域
    CvRect rect_test1, rect_test2,rect_test3,rect_test4;//字符方向顺序区域
    rect1=rects[position%4];
    rect2=rects[(position+1)%4];
    rect3=rects[(position+2)%4];
    rect4=rects[(position+3)%4];
    CvRect rect_temps[4]={rect1,rect2,rect3,rect4};//字符方向的模板区域
    rect_test1=rect1; rect_test2=rect2;rect_test3=rect3;rect_test4=rect4;
    CvRect rect_tests[4]={rect_test1, rect_test2,rect_test3,rect_test4};//字符方向的测试图匹配区域（加了margin）

    int margin = rect_top.height/10;

    /*根据字符位置分别对原区域进行margin裁剪*/
    if(position%2==0){
        for(int i=0;i<3;i+=2){
            rect_tests[i].x = rect_tests[i].x+2*margin;
            rect_tests[i].y = rect_tests[i].y+margin;
            rect_tests[i].width = rect_tests[i].width-4*margin;
            rect_tests[i].height= rect_tests[i].height-2*margin;

            rect_tests[i+1].x = rect_tests[i+1].x+margin;
            rect_tests[i+1].y = rect_tests[i+1].y+2*margin;
            rect_tests[i+1].width = rect_tests[i+1].width-2*margin;
            rect_tests[i+1].height= rect_tests[i+1].height-4*margin;
        }
        
    }else{
        for(int i=0;i<3;i+=2){
            rect_tests[i].x = rect_tests[i].x+margin;
            rect_tests[i].y = rect_tests[i].y+2*margin;
            rect_tests[i].width = rect_tests[i].width-2*margin;
            rect_tests[i].height= rect_tests[i].height-4*margin;

            rect_tests[i+1].x = rect_tests[i+1].x+2*margin;
            rect_tests[i+1].y = rect_tests[i+1].y+margin;
            rect_tests[i+1].width = rect_tests[i+1].width-4*margin;
            rect_tests[i+1].height= rect_tests[i+1].height-2*margin;
        }
    }
    /*将匹配区域写入model.txt*/
    fstream file;
    try{
        file.open(MODEL_PATH+"/model.txt", ios::out);
        file.clear();//先清空文件
    }catch (exception &e){
        cerr<<e.what()<<endl;
        cerr<<"模板文件打开失败!"<<endl;
        exit(0);
    }
    for(int i=0;i<4;i++){
        file << "待匹配区域的参数"+to_string(i+1)+":" << rect_tests[i].x << "," << rect_tests[i].y << "," << rect_tests[i].width << "," << rect_tests[i].height << endl;
    }
    file << "圆心参数:"<<center.x<<","<<center.y<<endl;
    file.close();
    
    //获取原图像
    unsigned int index = 1;
    unsigned int total = 4;
    char filename[100];
    //放模板匹配区域的图像
    vector<IplImage*> modelImages;
    //原图像
    IplImage* srcImage;
    //读取文件夹中的图像
    for (index; index <= total; index++){
        sprintf(filename, (TEST_PATH+btn_r+'/'+to_string(index)+".jpg").c_str());
        srcImage = cvLoadImage(filename, 1);

        if (srcImage != NULL){
            //灰度处理
            Mat blurModelDstMat = pertImage(srcImage);
            IplImage temp_1 = (IplImage)blurModelDstMat;

            IplImage* blurImage = &temp_1 ;
            IplImage* matchImage ;
            //存入容器中
            matchImage = getRectImage(blurImage, rect_temps[index-1].x, rect_temps[index-1].y, rect_temps[index-1].width, rect_temps[index-1].height);
            modelImages.push_back(matchImage);

            rectangle(blurModelDstMat,rect_temps[index-1],Scalar(0,0,255),2,1,0);  //在模板中画矩形框 并保存
            imwrite((RECT_PATH+to_string(index)+".jpg").c_str(), blurModelDstMat);
        }
        else{
            cout << "模板图像不足！" << endl;
            break;
        }
    }
    //取出匹配区域进行二值化并保存，其实可以不用存入容器中的，直接在上面二值化保存就行
    unsigned int vect_index = 0;
    for (vector<IplImage*>::iterator it = modelImages.begin(); it != modelImages.end(); it++)
    {
        Mat src = cvarrToMat(*it);
        sprintf(filename, (MODEL_PATH+"/"+to_string(vect_index)+".jpg").c_str());
        cout<<filename<<endl;
        if(GAUSS_DIFF)
            src=gauss_diff(src);//高反差
        imwrite(filename, src);
        vect_index = vect_index + 1;
    }

    cout <<"模板生成成功！" << endl;
    return 0;
}





int match(string btn_r,int symmetry,int position){
    Mat frame;
    int nbyte=0;
    string match_num;

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
        cerr<<"模板不存在或打开模板文件失败！"<<endl;
        exit(0);
    }

	string data[5];
    for(int i=0;i<5;i++){
        fileinput>>data[i];
    }
	fileinput.close();

	//待匹配区域的坐标
    Point center;
    string matchRectValue;
    vector<string> matchs;
    CvRect rect_tmp;
    CvRect rects[4]={};//4个区域
    for(int i=0;i<4;i++){
        matchRectValue = exchange(data[i], ":")[1];
        matchs = exchange(matchRectValue, ",");
        rect_tmp.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
        rect_tmp.y = atoi(matchs[1].c_str());
        rect_tmp.width = atoi(matchs[2].c_str());
        rect_tmp.height = atoi(matchs[3].c_str());
        rects[i]=rect_tmp;
        cout<<rect_tmp.x<<" "<<rect_tmp.y<<" "<<rect_tmp.width<<" "<<rect_tmp.height<<endl;
    }
    matchRectValue = exchange(data[4], ":")[1];
    matchs = exchange(matchRectValue, ",");
    center.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
    center.y = atoi(matchs[1].c_str());
    cout<<center.x<<" "<<center.y<<endl;

    //读入图片到容器中
	Mat modelMat;
	unsigned int index = 0;
	unsigned int total = 4;
	char filename[100];
	vector<Mat> modelMats;
	IplImage* matchImage;
	for (index; index < total; index++){
            sprintf(filename, (MODEL_PATH+"/"+to_string(index)+".jpg").c_str());
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

    Mat blurMatchDstMat,dstMatchRect1,dstMatchRect2;
    vector<Mat> rotate_imgs,move_imgs;
    vector<float> values;//35张图的角度匹配值
    int direction = 1;//旋转方向 0-左  1-右
    int test_no = 1;//测试图编号

    float value=0,value_tmp=0;
    double max = 0;
    int angle = 0;
    int iter = 0;
    int rect_index=0;
    cv::Point matchLoc;
    clock_t start,finish;
    double total_time;

    ofstream outFile;
    outFile.open(TEST_PATH+btn_r+'/'+btn_r+"_data.csv", ios::out); // 打开模式可省略
    frame = imread((TEST_PATH+btn_r+'/'+to_string(test_no)+".jpg").c_str(), 1);
    namedWindow("frame",0);
    resizeWindow("frame",width/2,height/2);
    namedWindow("show",0);
    resizeWindow("show",width/2,height/2);

    float test_mean,k,b=0;
    float temp_mean = 32.3729;
    Mat mask;
    Mat dst = Mat::zeros(frame.size(), CV_8UC1);
    Mat show;
    while(1){
        direction=1;
        waitKey(5);
        max = 0;
        angle = 0;
        iter = 0;
        if(test_no<=80)
        {
            cout<<test_no<<endl;
            frame = imread((TEST_PATH+btn_r+'/'+to_string(test_no++)+".jpg").c_str(), 1);
            imshow("frame", frame);
            blurMatchDstMat = pertImage0(frame);
            //线性变换
//            test_mean = mean(blurMatchDstMat,mask)(0);
//            k = temp_mean/test_mean;
//            lineTran(blurMatchDstMat,dst,k,b);
//            blurMatchDstMat=dst.clone();
//            imshow("line", blurMatchDstMat);

            rotate_imgs = rotation(blurMatchDstMat,center); //旋转后7张图像
            start = clock();
            for (vector <Mat>::iterator it = modelMats.begin(); it != modelMats.end(); it++){
                cout<<endl;
                rect_index = distance(modelMats.begin(),it);

                show=frame.clone();
                rectangle(show,rects[rect_index],Scalar(0,0,255),2,1,0);  //在模板中画矩形框 并保存
                imshow("show",show);
                cvWaitKey(5);

                move_imgs = move(rotate_imgs,rects[rect_index],(rect_index+position)%2);//rect旋转平移后35张
                for (vector <Mat>::iterator r1 = move_imgs.begin(); r1 != move_imgs.end(); r1++){//35张top
                    value_tmp = temp_match(*it, *r1, matchLoc, cv::TM_CCORR_NORMED);
                    values.push_back(value_tmp);
                    printf("%d=>%f\n ", iter, value_tmp);
                    imshow("test",*r1);
                    cvWaitKey(20);
//                    if(test_no==24){
//                        imwrite(("20180706/"+to_string(rect_index)+".jpg").c_str(),*r1);
//                    }
                }
                value = *max_element(values.begin(), values.end());
                printf("%d angle,最大值 = %f\n ", iter, value);

                if (max<value){
                    max = value;
                    angle = iter;
                }
                iter+=90;

                move_imgs.clear();
                values.clear();
            }
            finish = clock();
            total_time = (double)((finish-start)*1000/CLOCKS_PER_SEC);//ms
//            printf("match run time = %gms\n", total_time);//毫秒
//            if (angle==270){
//                angle=90;
//                direction = 0;
//            }
            if(angle-((test_no-2)%4)*90==0 || (abs(angle-((test_no-2)%4)*90)==180 && symmetry))
                outFile <<to_string(test_no-1)+".jpg"<< ',' << angle << endl;
            else
                outFile <<to_string(test_no-1)+".jpg"<< ',' << angle<<" error" << endl;
            max>=0 ? (direction==0? printf("旋转角度 = %d ,方向 左\n\n", angle):printf("旋转角度 = %d ,方向 右\n\n", angle)) : printf("无法识别\n");

        }
        else if(test_no>=100 || nbyte==4 && DATA_REC[1]==0xFE)//退出识别
        {
            outFile.close();
            return 0;
        } else{outFile.close();return 0;}
    }
}

int main()
{
    int symmetry;//对称
    int holes;//扣眼数
    int position;//字符位置（0上 1左 2下 3右）
    string radius;//纽扣半径
    cout<<"请输入纽扣半径,对称性（0-1）和字符位置（0123）：";
    cin>>radius>>symmetry>>position;
    generate_temp(radius,symmetry,position);
    match(radius,symmetry,position);
    return 0  ;
}

