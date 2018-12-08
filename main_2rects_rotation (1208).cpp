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


int generate_temp(string btn_r){
    int num = 0,cur_no=0,sum=4;  //num当前最大模板编号  sum采集模板图片数（4） cur_no当前采集模板编号
    char key;
    //删除原模板文件
    DIR* dir = opendir(MODEL_PATH.c_str());//打开指定目录
    dirent* p = NULL;//定义遍历指针
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

    //从摄像头读取4个角度模板原图.保存至20180706/
    Mat frame;

    namedWindow("frame",1);
    destroyWindow("frame");

    /*
     * 计算区域的参数
     */
    double button_radius = atof(btn_r.c_str()); //纽扣半径 mm

    Point center = Point(660,533); //纽扣中心
    double section_angle = PI/3; //每一个截取区域的角度 60度
    double pixs_mm = 18.43;//单位长度对应像素距离 = 18.53pixs/mm
    int left_x = 595;//4个铁钉外接矩形左边缘x
    int top_y  = 470;//4个铁钉外接矩形上边缘y
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


    //先清空文件
    fstream file;
    file.open(MODEL_PATH+"/model.txt", ios::out);
    if (!file.is_open()){
        cout << "模板文件打开失败!" << endl;
        return 0;
    }
    file.clear();

    int margin = rect1.height/10;
    //待匹配区域为手选区域zhongjian部分的边缘缩进4 pixs
    file << "待匹配区域的参数1:" << rect1.x+2*margin << "," << rect1.y+margin << "," << rect1.width-4*margin << "," << rect1.height-2*margin << endl;
    file << "待匹配区域的参数2:" << rect2.x+margin << "," << rect2.y+2*margin << "," << rect2.width-2*margin << "," << rect2.height-4*margin << endl;
    file << "圆心参数:"<<center.x<<","<<center.y<<endl;

    file.close();
    //获取原图像
    unsigned int index = 1;
    unsigned int total = 4;
    char filename[100];
    //放模板匹配区域的图像
    vector<IplImage*> modelImages;
    //原图像
    IplImage* srcImage; /* = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);*/
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


//获取旋转后图像
vector<Mat> rotation(Mat img,Point center)
{
    vector<Mat> rotation_imgs;
    Mat map_matrix,res,dst;

    namedWindow("rotation",0);
    resizeWindow("rotation",width/2,height/2);

    for(int i=-6;i<=6;i+=2){ //7度
        //map_matrix=getRotationMatrix2D(center,i,1.0);  //旋转中心，角度，缩放比例
        map_matrix=getRotationMatrix2D(center, i, 1.0);  //旋转中心，角度，缩放比例
        warpAffine(img,dst,map_matrix,Size(img.cols,img.rows));
        res = dst.clone();
        rotation_imgs.push_back(res);

        imshow("rotation",res(Rect(img.cols*(1.0/3)/2,img.rows*(1.0/3)/2,(2.0/3)*img.cols,(2.0/3)*img.rows)));
        cvWaitKey(5);
    }


    return rotation_imgs;
}

//获取平移后图像
vector<Mat> move(vector<Mat> rotate_imgs,Rect rect,string flag){
    Mat res;
    vector<Mat> move_imgs;
    Rect match=rect;
    for(vector<Mat>::iterator it=rotate_imgs.begin();it!=rotate_imgs.end();it++){//7张旋转图
        for(int i=-10;i<=10;i+=5){//5张平移
            if(flag=="top")
                {match.y=rect.y+i;
                cout<<"top"<<endl;}
            else
                {match.x=rect.x+i;
                cout<<"left"<<endl;}
            res = (*it)(match).clone();
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


int match(string btn_r,int symmetry){
    char key;
    Mat frame;
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
        cout<<"模板不存在或打开模板文件失败！"<<endl;
        return 0;
    }
	string data[4];
	fileinput >> data[0];
	fileinput >> data[1];
    fileinput >> data[2];
	fileinput.close();

	//待匹配区域的坐标
    Point center;
	CvRect matchRect1, matchRect2;

	string matchRectValue = exchange(data[0], ":")[1];
	vector<string> matchs = exchange(matchRectValue, ",");
	matchRect1.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
	matchRect1.y = atoi(matchs[1].c_str());
	matchRect1.width = atoi(matchs[2].c_str());
	matchRect1.height = atoi(matchs[3].c_str());
    cout<<matchRect1.x<<" "<<matchRect1.y<<" "<<matchRect1.width<<" "<<matchRect1.height<<endl;

    matchRectValue = exchange(data[1], ":")[1];
    matchs = exchange(matchRectValue, ",");
    matchRect2.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
    matchRect2.y = atoi(matchs[1].c_str());
    matchRect2.width = atoi(matchs[2].c_str());
    matchRect2.height = atoi(matchs[3].c_str());
    cout<<matchRect2.x<<" "<<matchRect2.y<<" "<<matchRect2.width<<" "<<matchRect2.height<<endl;

    matchRectValue = exchange(data[2], ":")[1];
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
	IplImage* matchImage = cvCreateImage(cvSize(matchRect1.width, matchRect1.height), IPL_DEPTH_8U, 1);
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

    IplImage* matchRectImage1, *matchRectImage2;
    IplImage matchBlurImage;
    Mat blurMatchDstMat,dstMatchRect1,dstMatchRect2/*,frame*/;
    vector<Mat> move_imgs,rotate_imgs,move_rect1_imgs,move_rect2_imgs;
    vector<float> values1,values2;//35张图的角度匹配值
    int direction = 1;//旋转方向 0-左  1-右
    int nbyte = 0;
    int test_no = 1;//测试图编号

    float value1 = 0,value2=0,avg_value=0;
    double max = 0;
    int angle = 0;
    int iter = 0;
    cv::Point matchLoc;
    //逐一匹配
//            double Time = (double)cvGetTickCount();
    clock_t start,finish;
    double total_time;

    ofstream outFile;
    outFile.open(TEST_PATH+btn_r+'/'+btn_r+"_data.csv", ios::out); // 打开模式可省略

    frame = imread((TEST_PATH+btn_r+'/'+to_string(test_no)+".jpg").c_str(), 1);
    namedWindow("frame",0);
    resizeWindow("frame",width/2,height/2);

    float test_mean,k,b=0;
    float temp_mean = 32.3729;
    Mat mask;
    Mat dst = Mat::zeros(frame.size(), CV_8UC1);

    while(1){
        direction=1;
        key = waitKey(5);
        imshow("frame", frame);

        if(test_no<=80)
        {
            cout<<test_no<<endl;
            frame = imread((TEST_PATH+btn_r+'/'+to_string(test_no++)+".jpg").c_str(), 1);
            blurMatchDstMat = pertImage0(frame);
            test_mean = mean(blurMatchDstMat,mask)(0);
            k = temp_mean/test_mean;
            lineTran(blurMatchDstMat,dst,k,b);
            blurMatchDstMat=dst.clone();

            rectangle(frame,matchRect1,Scalar(0,0,255),2,1,0);
            rectangle(frame,matchRect2,Scalar(0,0,255),2,1,0);
            imshow("frame", frame);
            imshow("line", blurMatchDstMat);

            move_imgs.clear();
            rotate_imgs = rotation(blurMatchDstMat,center); //旋转后7张图像
            move_rect1_imgs = move(rotate_imgs,matchRect1,"top");//rect1旋转平移后35张
            move_rect2_imgs = move(rotate_imgs,matchRect2,"left");//rect2旋转平移后35张




//            matchBlurImage = (IplImage)blurMatchDstMat;
//
//            //截匹配区域
//            matchRectImage1 = getRectImage(&matchBlurImage, matchRect1.x, matchRect1.y, matchRect1.width, matchRect1.height);
//            matchRectImage2 = getRectImage(&matchBlurImage, matchRect2.x, matchRect2.y, matchRect2.width, matchRect2.height);
//            dstMatchRect1 = cvarrToMat(matchRectImage1);
//            dstMatchRect2 = cvarrToMat(matchRectImage2);

            avg_value=0,value1 = 0,value2=0;
            max = 0;
            angle = 0;
            iter = 0;
            //逐一匹配
            start = clock();

            for (vector <Mat>::iterator it = modelMats.begin(); it != modelMats.end(); it+=2){
                cout<<endl;
                //top rect
                for (vector <Mat>::iterator r1 = move_rect1_imgs.begin(); r1 != move_rect1_imgs.end(); r1++){//35张top
                    value1 = temp_match(*it, *r1, matchLoc, cv::TM_CCORR_NORMED);
                    values1.push_back(value1);
                    printf("top %d angle,value = %f\n ", iter, value1);

                    imshow("top",*r1);
                    cvWaitKey(10);
                }
                value1 = *max_element(values1.begin(), values1.end());
                printf("top %d angle,最大value = %f\n ", iter, value1);


                //left rect
                for (vector <Mat>::iterator r2 = move_rect2_imgs.begin(); r2 != move_rect2_imgs.end(); r2++){//35张left
                    value2 = temp_match(*(it+1), *r2, matchLoc, cv::TM_CCORR_NORMED);
                    values2.push_back(value2);
                    printf("left %d angle,value = %f\n ", iter, value2);

                    imshow("left",*r2);
                    cvWaitKey(10);
                }
                value2 = *max_element(values2.begin(), values2.end());
                printf("left %d angle,最大value = %f\n ", iter, value2);


                avg_value = (value1+value2)/2;
                printf("this is %d angle, final value = %f\n ", iter, avg_value);
                if (max<avg_value){
                    max = avg_value;
                    angle = iter;
                }
                iter+=90;
                values1.clear();
                values2.clear();
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
    int symmetry;
    string r;
    cout<<"请输入纽扣半径和是否对称（0-1）：";
    cin>>r>>symmetry;
    cout<<r<<" d"<<symmetry;
    generate_temp(r);
    match(r,symmetry);
    return 0  ;
}

