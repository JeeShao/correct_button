//#include<opencv2/core/core.hpp>
//#include<opencv2/highgui/highgui.hpp>
//#include <opencv2/core/core.hpp>
//#include<opencv2/opencv.hpp>
//#include <fstream>
//#include <dirent.h>
//#include <sys/stat.h>
//#include <vector>
//#include <sys/types.h>
//#include<iostream>
//#include<string>
//#include<time.h>
#include <math.h>
#include <pthread.h>
#include "rect_image.h"
#include "match.h"
#include "serial.h"
#include "JHCap.h"
#include "bmpfile.h"
#include "common.h"

//using namespace std;
//using namespace cv;

#define PI 3.1415926
//#define GAUSS_DIFF 1//高反差
#define LAPLACE 0//拉普拉斯增强
#define MOVE_PIXS 3
#define INIT 0 //初始化
#define BLUR 1 //模糊
#define THRESH 1 //自适应
#define RECT_PRECENT  3.0/4  //矩形占比2/3

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

/*************************************
Function:    generate_temp()
Description: 模板生成
Parameters:  btn_r:纽扣半径
             symmetry:对称性
             position:字符位置
Return:      int
**************************************/
int generate_temp(){
    cout<<"进入模板生成程序……"<<endl;
    int num=0,cur_no=0;char key;
    string btn_r="15.30";int holes=4,position=3;
    //删除原模板文件
//    if(!remove_files(MODEL_PATH.c_str()))
//        {cerr<<"删除原模板文件失败"<<endl; exit(0);}
    //从摄像头读取4个角度模板原图.保存至20180706/
    Mat frame;

    DATA_MSG[2] = 03;  //完成
    sendnTTY(ptty,DATA_MSG,5);//准备好

//    recvnTTY(ptty,DATA_REC,5);
//    for(int i=0;i<5;i++)
//        printf("%02X ",DATA_REC[i]);
//    cout<<endl;
//    cout<<"串口数据: "<<DATA_REC[0]<<'-'<<DATA_REC[1]<<'-'<<DATA_REC[2]<<'-'<<DATA_REC[3]<<'-'<<DATA_REC[5]<<endl;

//    if(DATA_REC[1]==0xFC){ //纽扣半径
//        btn_r = DATA_REC[2];
//    }
//    memset(DATA_REC,0,8);

//    recvnTTY(ptty,DATA_REC,5);
//    for(int i=0;i<5;i++)
//        printf("%02X ",DATA_REC[i]);
//    cout<<endl;
//
////    cout<<"串口数据: "<<DATA_REC[0]<<'-'<<DATA_REC[1]<<'-'<<DATA_REC[2]<<'-'<<DATA_REC[3]<<'-'<<DATA_REC[5]<<endl;
//    if(DATA_REC[1]==0xF8){ //纽扣扣眼数
//        holes = DATA_REC[2];
//    }
//    memset(DATA_REC,0,8);

    /*
     * 计算区域的参数
     */
    double button_radius = atof(btn_r.c_str()); //纽扣半径 mm
    Point center = Point(660,533); //纽扣中心
    double section_angle = PI/3; //每一个截取区域的角度 60度
    double pixs_mm = 18.43;//单位长度对应像素距离 = 18.53pixs/mm
    int left_x = 595;//4个铁钉外接矩形左边缘x
    int top_y  = 470;//4个铁钉外接矩形上边缘y
    int bottom_y = center.y+center.y-top_y;//4个铁钉外接矩形下边缘y
    int right_x = center.x+center.x-left_x;//4个铁钉外接矩形右边缘x
    int R = button_radius*pixs_mm; //纽扣像素半径
    if (INIT){
        namedWindow("frame",0);
        resizeWindow("frame",width/2,height/2);
        while(1){
            key=cvWaitKey(10);
            if(CameraQueryImage(0,(unsigned char*)image->imageData, &len, CAMERA_IMAGE_BMP/* |CAMERA_IMAGE_TRIG*/)==API_OK)
            {
                frame = cvarrToMat(image);
                imshow("frame",frame);
                if(key==32)
                {init(frame,button_radius);break;}
            }
        }
    }
    ifstream fileinput;
    try {
        fileinput.open(INIT_FILE);
    }catch ( exception &e){
        cerr << "Caught: " << e.what( ) << endl;
        cerr << "Type: " << typeid( e ).name( ) << endl << endl;
    }
    if (!fileinput.is_open())
    {
        cerr<<"打开初始化文件失败！"<<endl;
        exit(0);
    }

    string data[6];
    for(int i=0;i<6;i++){
        fileinput>>data[i];
    }
    fileinput.close();

    string initValue;
    vector<string> matchs;

    initValue = exchange(data[0], ":")[1];
    matchs = exchange(initValue, ",");
    center.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
    center.y = atoi(matchs[1].c_str());

    top_y = atoi((exchange(data[1], ":")[1]).c_str());
    left_x = atoi((exchange(data[2], ":")[1]).c_str());
    bottom_y = atoi((exchange(data[3], ":")[1]).c_str());
    right_x = atoi((exchange(data[4], ":")[1]).c_str());
    pixs_mm = atof((exchange(data[5], ":")[1]).c_str());

    R = button_radius*pixs_mm;

    /* 自动计算4块截取区域
     * 矩形顶点坐标可以适当往里面缩进一些，宽度和高度也可以适当减小，对宽高设置阈值判断 增加容错
     */
    CvRect rect_top, rect_left,rect_bottom,rect_right;//上下左右区域
    //top area
    rect_top.x = center.x-R*sin(section_angle/2);
    rect_top.y = center.y-R*cos(section_angle/2);
    rect_top.width = 2*R*sin(section_angle/2);
    rect_top.height = RECT_PRECENT*(top_y-rect_top.y);//区域高度设为区域顶部到铁钉上边缘的2/3
    //left area
    rect_left.x = center.x-R*cos(section_angle/2);
    rect_left.y = center.y-R*sin(section_angle/2);
    rect_left.width = RECT_PRECENT*(left_x-rect_left.x);//区域宽度设为区域左部到铁钉左边缘的2/3
    rect_left.height = 2*R*sin(section_angle/2);
    //bottom area
    rect_bottom.x = center.x-R*sin(section_angle/2);
    rect_bottom.y = bottom_y+(1.0-RECT_PRECENT)*(center.y+R*cos(section_angle/2)-bottom_y);
    rect_bottom.width = 2*R*sin(section_angle/2);
    rect_bottom.height = center.y+R*cos(section_angle/2)-rect_bottom.y;// 2*(rect_bottom.y-bottom_y);
    //right area
    rect_right.x = right_x+(1.0-RECT_PRECENT)*(center.x+R*cos(section_angle/2)-right_x);
    rect_right.y = center.y-R*sin(section_angle/2);
    rect_right.width = center.x+R*cos(section_angle/2)-rect_right.x;// 2*(rect_right.x-right_x);//区域宽度设为区域左部到铁钉左边缘的2/3
    rect_right.height = 2*R*sin(section_angle/2);
    
    /*根据字符位置position确定区域顺序*/
//    CvRect rects[4]={rect_top,rect_left,rect_bottom,rect_right};
    CvRect rects[4]={rect_top,rect_right,rect_bottom,rect_left};
    CvRect rect1, rect2,rect3,rect4;//字符方向顺序区域
    CvRect rect_test1, rect_test2,rect_test3,rect_test4;//字符方向顺序区域
    rect1=rects[position%4];
    rect2=rects[(position+1)%4];
    rect3=rects[(position+2)%4];
    rect4=rects[(position+3)%4];
    CvRect rect_temps[4]={rect1,rect2,rect3,rect4};//字符方向的模板区域
    rect_test1=rect1; rect_test2=rect2;rect_test3=rect3;rect_test4=rect4;
    CvRect rect_tests[4]={rect_test1, rect_test2,rect_test3,rect_test4};//字符方向的测试图匹配区域（加了margin）
//    int margin = rect_top.height/10;
    int margin = 10;//4;


    /*根据字符位置分别对原区域进行margin裁剪*/
    if(holes==4){
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
    } else if(holes==2) {
        rect_temps[1]=rect3;//字符方向的模板区域
        if (position % 2 == 0) {
            for (int i = 0; i < 3; i += 2) {
                rect_tests[i].x = rect_tests[i].x + 2 * margin;
                rect_tests[i].y = rect_tests[i].y + margin;
                rect_tests[i].width = rect_tests[i].width - 4 * margin;
                rect_tests[i].height = rect_tests[i].height - 2 * margin;

                rect_tests[i + 2].x = rect_tests[i + 2].x + margin;
                rect_tests[i + 2].y = rect_tests[i + 2].y + 2 * margin;
                rect_tests[i + 2].width = rect_tests[i + 2].width - 2 * margin;
                rect_tests[i + 2].height = rect_tests[i + 2].height - 4 * margin;
            }
        } else {
            for (int i = 0; i < 3; i += 2) {
                rect_tests[i].x = rect_tests[i].x + margin;
                rect_tests[i].y = rect_tests[i].y + 2 * margin;
                rect_tests[i].width = rect_tests[i].width - 2 * margin;
                rect_tests[i].height = rect_tests[i].height - 4 * margin;

                rect_tests[i + 2].x = rect_tests[i + 2].x + 2 * margin;
                rect_tests[i + 2].y = rect_tests[i + 2].y + margin;
                rect_tests[i + 2].width = rect_tests[i + 2].width - 4 * margin;
                rect_tests[i + 2].height = rect_tests[i + 2].height - 2 * margin;
            }
        }
        rect_tests[1] = rect_tests[2];
        rect_tests[2]=0x0;
        rect_tests[3]=0x0;
    }else{cerr<<"扣眼参数输入错误！"<<endl;exit(0);}

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
    unsigned int index = 0;
    unsigned int total = holes;
    char filename[100];
    //放模板匹配区域的图像
    vector<Mat> modelImages;
    //原图像
    Mat srcImage;
    clock_t start,finish;
    namedWindow("src",0);
    resizeWindow("src",width/2,height/2);
    while(1){
        key = waitKey(10);
        if(CameraQueryImage(0,(unsigned char*)image->imageData, &len,
                            CAMERA_IMAGE_BMP/* |CAMERA_IMAGE_TRIG*/)==API_OK)
        {
            frame = cvarrToMat(image);
            imshow("src", frame);

            if(recvnTTY(ptty,DATA_REC,4)==4)//拍照
            {
//                start = clock();
                for(int i=0;i<4;i++)
                printf("%02X ",DATA_REC[i]);
                cout<<endl;

//                printf("%02x\n",DATA_REC[1]);
                if(DATA_REC[1]==0xFD)//拍照
                {
                    if(sendnTTY(ptty,DATA_MSG,5)==5){ //完成
                        cout<<"模板图："<<cur_no<<endl;
                        imwrite((ORG_PATH+to_string(cur_no++)+".jpg").c_str(),frame);//图片保存到本工程目录中
                    }else
                        cout<<"send code error";
                }
                else if(DATA_REC[1]==0xFE)  //退出模板
                {cout<<"退出模板"<<endl;break;}
                else if(DATA_REC[1]==0xFA)
                {cerr<<"模板生成失败"<<endl;return 0;}
                memset(DATA_REC,0,8);
            }
        }
        if(cur_no>=holes)
            break;
    }
    cvWaitKey(500);
    destroyAllWindows();

    //读取文件夹中的图像
    for (index; index < total; index++){
        sprintf(filename, (ORG_PATH+'/'+to_string(index)+".jpg").c_str());
        srcImage = imread(filename, 1);

        if (!srcImage.empty()){
            //灰度处理
            Mat blurModelDstMat = pertImage0(srcImage);
            Mat matchImage ;
            //存入容器中
            matchImage = blurModelDstMat(rect_temps[index]).clone();

            modelImages.push_back(matchImage);

            rectangle(blurModelDstMat,rect_temps[index],Scalar(0,0,255),2,1,0);
            imwrite((RECT_PATH+to_string(index)+".jpg").c_str(), blurModelDstMat);
        }
        else{
            cout << "模板图像不足！" << endl;
            break;
        }
    }

    //取出匹配区域进行二值化并保存，其实可以不用存入容器中的，直接在上面二值化保存就行
    unsigned int vect_index = 0;
    for (vector<Mat>::iterator it = modelImages.begin(); it != modelImages.end(); it++)
    {
        Mat src = (*it).clone();
//        imshow("it",src);
        sprintf(filename, (MODEL_PATH+"/"+to_string(vect_index)+".jpg").c_str());
        cout<<filename<<endl;
        if(GAUSS_DIFF)
            src=gauss_diff(src);//高反差
        if(LAPLACE)
        {src=laplace_enhance(src);//拉普拉斯增强
        cvWaitKey(10);}
        if(THRESH)
            adaptiveThreshold(src,src,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,25,5);
        if(BLUR)
        {GaussianBlur(src,src,Size(5,5),0,0);GaussianBlur(src,src,Size(17,17),0,0);}
        imwrite(filename, src);
        vect_index = vect_index + 1;
    }
    destroyAllWindows();

    cout <<"模板生成成功！" << endl;
    return 0;
}





int match(){
    cout<<"进入方向识别程序……"<<endl;

    string btn_r="15.30";int holes=4,position=3;
    Mat frame;
    int num=0,nbyte=0;char key;
    DATA_MSG[2] = 03;  //完成
    sendnTTY(ptty,DATA_MSG,5);//准备好

//    recvnTTY(ptty,DATA_REC,5);
//    if(DATA_REC[1]==0xFC){ //纽扣半径
//        btn_r = DATA_REC[2];
//    }
//    memset(DATA_REC,0,8);

//    recvnTTY(ptty,DATA_REC,5);
//    if(DATA_REC[1]==0xF8){ //纽扣扣眼数
//        holes = DATA_REC[2];
//    }
//    memset(DATA_REC,0,8);

	//先读model.txt
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

    Point center;
    string matchRectValue;
    vector<string> matchs;
    CvRect rect_tmp;
    CvRect rects[holes]={};//4个区域
    for(int i=0;i<holes;i++){
        matchRectValue = exchange(data[i], ":")[1];
        matchs = exchange(matchRectValue, ",");
        rect_tmp.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
        rect_tmp.y = atoi(matchs[1].c_str());
        rect_tmp.width = atoi(matchs[2].c_str());
        rect_tmp.height = atoi(matchs[3].c_str());
        rects[i]=rect_tmp;
//        cout<<rect_tmp.x<<" "<<rect_tmp.y<<" "<<rect_tmp.width<<" "<<rect_tmp.height<<endl;
    }
    matchRectValue = exchange(data[4], ":")[1];
    matchs = exchange(matchRectValue, ",");
    center.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
    center.y = atoi(matchs[1].c_str());
//    cout<<center.x<<" "<<center.y<<endl;

    //读入图片到容器中
	Mat modelMat;
	unsigned int index = 0;
	unsigned int total = holes;
	char filename[100];
	vector<Mat> modelMats;
	Mat matchImage;
	for (index; index < total; index++){
            sprintf(filename, (MODEL_PATH+"/"+to_string(index)+".jpg").c_str());
            matchImage = imread(filename, 0);//读取灰度
            if (!matchImage.empty()){
                modelMats.push_back(matchImage);
            }
            else{ cerr<<"模板图像不足！" << endl;exit(0); }
	}

    Mat blurMatchDstMat,dstMatchRect1,dstMatchRect2;
    vector<Mat> rotate_imgs,move_imgs;
    vector<float> values;//35张图的角度匹配值
    int direction = 1;//旋转方向 0-左  1-右

    float value=0,value_tmp=0;
    double max = 0;
    int angle = 0;
    int iter = 0;
    int rect_index=0;
    cv::Point matchLoc;
    clock_t start,finish;
    double total_time;

    namedWindow("frame",0);
    resizeWindow("frame",width/2,height/2);


//    Mat show;
    int ii=0;
    while(1){
        direction=1;
        waitKey(5);
        if(CameraQueryImage(0,(unsigned char*)image->imageData, &len,
                            CAMERA_IMAGE_BMP/* |CAMERA_IMAGE_TRIG*/)==API_OK)
        {
            frame = cvarrToMat(image);
            imshow("frame",frame);

            memset(DATA_REC,0,8);  //清空
            nbyte = recvnTTY(ptty,DATA_REC,4);

            if(nbyte==4 && DATA_REC[1]==0xFD)  //拍照
            {
                start = clock();
                cout<<"识别：";
                for(int i=0;i<4;i++)
                    printf("%02X ",DATA_REC[i]);
                cout<<endl;
                imwrite(("识别照片/"+to_string(ii++)+".jpg").c_str(),frame);
//                namedWindow("show",0);
//                resizeWindow("show",width/2,height/2);
                max = 0;angle = 0;iter = 0;
                blurMatchDstMat = pertImage0(frame);
                rotate_imgs = rotation(blurMatchDstMat,center); //旋转后7张图像
//                show=frame.clone();
                for (vector <Mat>::iterator it = modelMats.begin(); it != modelMats.end(); it++){
//                    cout<<endl;
                    rect_index = distance(modelMats.begin(),it);

//                    rectangle(show,rects[rect_index],Scalar(0,0,255),2,1,0);  //在模板中画矩形框 并保存
//                    imshow("show",show);

                    move_imgs = move(rotate_imgs,rects[rect_index],(rect_index+position)%2);//rect旋转平移后35张
                    for (vector <Mat>::iterator r1 = move_imgs.begin(); r1 != move_imgs.end(); r1++){//35张top
                        if(LAPLACE)
                            *r1=laplace_enhance(*r1);//拉普拉斯增强
                        if(THRESH)
                            adaptiveThreshold(*r1,*r1,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,25,5);
                        if(BLUR)
                        {GaussianBlur(*r1,*r1,Size(5,5),0,0);GaussianBlur(*r1,*r1,Size(17,17),0,0);}
                        value_tmp = temp_match(*it, *r1, matchLoc,TM_CCOEFF_NORMED );//TM_CCORR_NORMED
                        values.push_back(value_tmp);
//                        printf("%d=>%f\n ", iter, value_tmp);
//                        imshow("匹配图",*r1);
                    }
//                    cvWaitKey(2000);
                    value = *max_element(values.begin(), values.end());
                    printf("%d angle,最大值 = %f\n ", iter, value);

                    if (max<value){ max = value;angle = iter; }
                    iter+=(360/holes);
                    move_imgs.clear();
                    values.clear();
                }
//            if (angle==270){angle=90;direction = 0;}
                max>=0 ? (direction==0? printf("旋转角度 = %d ,方向 左\n", angle):printf("旋转角度 = %d ,方向 右\n", angle)) : printf("无法识别\n");
                finish = clock();
                total_time = (double)((finish-start)*1000/CLOCKS_PER_SEC);//ms
                printf("识别时间 = %gms\n\n", total_time);//毫秒

                if(max>=0.6){
                    DATA_ANGLE[2] = direction;
                    DATA_ANGLE[3] = angle;
                    sendnTTY(ptty,DATA_ANGLE,6);
                }else{//01图像无法使别
                    DATA_MSG[2] = 01;
                    sendnTTY(ptty,DATA_MSG,5);
                }
            }
            else if(nbyte==4 && DATA_REC[1]==0xFE){//退出识别
                return 0;
            }
            else if(nbyte>0){
                for(int i=0;i<4;i++)
                    printf("%02X ",DATA_REC[i]);
                cout<<endl;
                cerr<<"异常指令"<<endl;
//                return 0;
            }
        }
    }
}

void* show(void*)
{
    Mat frame;
    namedWindow("实时",0);
    resizeWindow("实时",width/4,height/4);
//    pthread_detach(pthread_self());
    while(1){
        if(CameraQueryImage(0,(unsigned char*)image->imageData, &len, CAMERA_IMAGE_BMP/* |CAMERA_IMAGE_TRIG*/)==API_OK)
        {
            frame = cvarrToMat(image);
            imshow("实时",frame);
        }
        cvWaitKey(10);
    }
}

int main()
{
    int count,select;
    int symmetry;//对称
    int holes;//扣眼数
    int position;//字符位置（0上 1左 2下 3右）
    string radius;//纽扣半径

    //初始化相机
    CameraGetCount(&count);
    printf("Camera count: %d\n", count);
    if(count<1) return 0;
    CameraInit(0);
    CameraSetGamma(0,1.33);//咖马值
    CameraSetContrast(0,1);//对比度
    CameraSetSaturation(0,1.18);//饱和度
    CameraSetBlackLevel(0,10);//黑电平
    CameraSetGain(0, 255);//增益150
    CameraSetExposure(0, 1200);//曝光800
//    CameraSetOption(0,CAMERA_IMAGE_RGB24);//图像格式

    CameraSetSnapMode(0, CAMERA_SNAP_CONTINUATION);
    CameraSetResolution(0, 2, &width, &height);
    CameraGetImageBufferSize(0, &len, CAMERA_IMAGE_BMP);

    //初始化串口
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
    fcntl(ptty->fd, F_SETFL, FNDELAY);//非阻塞
//    fcntl(ptty->fd, F_SETFL, 0);//阻塞

    //显示实时画面
//    pthread_t id;
//    int ret = pthread_create(&id, NULL, show, NULL);
////    pthread_detach(id);
//
//    if(ret) {
//        cout << "Create pthread error!" << endl;
//        return 1;
//    }

//    while(1){
//        cout<<"等待扣眼和半径数据……"<<endl;
//        if(recvnTTY(ptty,DATA_REC,5)==5){ //扣眼 半径
//            for(int i=0;i<5;i++)
//                printf("%02X ",DATA_REC[i]);
//            cout<<endl;
//        }
//        memset(DATA_REC,0,8);
//        cvWaitKey(10);
//    }


    while(1){
        fcntl(ptty->fd, F_SETFL, 0);//阻塞
        cout<<"等待串口数据……"<<endl;
        cvWaitKey(10);


        if(recvnTTY(ptty,DATA_REC,4)==4) {
            fcntl(ptty->fd, F_SETFL, FNDELAY);//非阻塞

//            cout.unsetf(ios_base::dec);
//            cout.setf(ios_base::hex);
//            cout<<"串口数据: "<<hex<<DATA_REC[0]<<'-'<<DATA_REC[1]<<'-'<<DATA_REC[2]<<'-'<<DATA_REC[3]<<endl;
//            cout << DATA_REC[1] << endl;
            for(int i=0;i<4;i++)
                printf("%02X ",DATA_REC[i]);
            cout<<endl;

            switch (DATA_REC[1]) {
                case 0xFF:{generate_temp();break;}
                case 0xFA:{match();break;}
                default:{match();cerr<<"指令错误！默认进入识别模式"<<endl;break;}
            }
        }
    }
}

