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
const string TEST_PATH = "pp/";//测试图像文件路径
const string RECT_PATH = FILE_PATH+"org_rects/"; //模板图矩形框

TTY_INFO *ptty;  //串口
unsigned char DATA_MSG[] = {0xDE,0xA9,00,0xFF,0xFF};  //通知
unsigned char DATA_ANGLE[] = {0xDE,0xA1,01,00,0xFF,0xFF}; //旋转角度
unsigned char DATA_WIDTH[] = {0xDE,0xFC,00,0xFF,0xFF}; //纽扣宽度
unsigned char DATA_REC[8] = {}; //接受串口信息

int width=1280, height=1024, len=0;
IplImage *image = cvCreateImage(cvSize(width, height), 8, 3);
Point start1 = Point(640,0);
Point end1 = Point(640,1024);
Point start2 = Point(0,647);
Point end2 = Point(1280,647);


vector<Mat> rotation(Mat img,Rect outter,Point center)
{
    vector<Mat> rotation_imgs;
    Mat map_matrix,dst;
    Mat src(img.rows,img.cols,CV_8UC1);
    for(int i=-3;i<=3;i++){
        map_matrix=getRotationMatrix2D(center,i,1.0);  //旋转中心，角度，缩放比例
        warpAffine(img,src,map_matrix,Size(img.cols,img.rows));
        dst = src(outter).clone();
//        imshow("src",dst);
//        cvWaitKey(2000);
        rotation_imgs.push_back(dst);
    }
    return rotation_imgs;
}

int generate_temp(string btn_r){
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

    /*
     * 计算区域的参数
     */
    double button_radius = atof(btn_r.c_str()); //纽扣半径 mm
    Point center = Point(710,594); //纽扣中心
    double section_angle = PI/3; //每一个截取区域的角度 60度
    double pixs_mm = 18.53;//单位长度对应像素距离 = 18.53pixs/mm
    int R = button_radius*pixs_mm; //纽扣像素半径
    int r = sqrt(pow((center.x-610),2)+pow((center.y-500),2)); //内接圆半径
    int margin = 0; //模板边距
    //内部矩形
//    CvRect rect_inner;
//    rect_inner.x = 620;
//    rect_inner.y = 500;
//    rect_inner.width = 2*(center.x-rect_inner.x);//内部矩形宽度
//    rect_inner.height = 2*(center.y-rect_inner.y);//内部矩形高度
    //纽扣外接矩
    CvRect rect_outter;
    rect_outter.x = center.x-R;
    rect_outter.y = center.y-R;
    rect_outter.width = 2*R;
    rect_outter.height = 2*R;

    //先清空文件
    fstream file;
    file.open(MODEL_PATH+"/model.txt", ios::out);
    if (!file.is_open()){
        cout << "模板文件打开失败!" << endl;
        return 0;
    }
    file.clear();

    //待匹配区域为手选区域zhongjian部分的边缘缩进4 pixs
    file << "内接圆参数:" << center.x<< "," << center.y<< "," << r << endl;
    file << "外接矩参数:" << rect_outter.x+margin << "," << rect_outter.y+margin << "," << rect_outter.width-2*margin << "," << rect_outter.height-2*margin << endl;
    file << "外接圆参数:" << center.x<< "," << center.y<< "," << R-15<< endl;

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

            //截取外圆
            Mat img = Mat::zeros(blurModelDstMat.size(), CV_8UC1);
            Mat mask = Mat::zeros(blurModelDstMat.size(), CV_8UC1);
            circle(mask, center, R-15, Scalar(255), -1, 8);
            blurModelDstMat.copyTo(img, mask);
//            img.copyTo(blurModelDstMat,mask);
            //截取内圆
            blurModelDstMat = img.clone();
            img.setTo(Scalar(0));
            mask.setTo(Scalar(0));
            circle(mask, center, r, Scalar(255), -1, 8);
            img.copyTo(blurModelDstMat,mask);

//            Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
//            erode(blurModelDstMat,blurModelDstMat,element);//腐蚀

//            blurModelDstMat(rect_inner).setTo(0);//将内接矩像素置为0
            IplImage temp_1 = (IplImage)blurModelDstMat;

            IplImage* blurImage = &temp_1 ;
            IplImage* matchImage ;
            //存入容器中
            matchImage = getRectImage(blurImage, rect_outter.x, rect_outter.y, rect_outter.width, rect_outter.height);
            modelImages.push_back(matchImage);

            rectangle(blurModelDstMat,rect_outter,Scalar(0,0,255),2,1,0);
            imwrite((RECT_PATH+to_string(index)+".jpg").c_str(), blurModelDstMat);
        }
        else{
            cout << "模板图像不足！" << endl;
            break;
        }
    }
    //取出匹配区域进行二值化并保存，其实可以不用存入容器中的，直接在上面二值化保存就行
    int vect_index=0;
    for (vector<IplImage*>::iterator it = modelImages.begin(); it != modelImages.end(); it++,vect_index++)
    {
        Mat src = cvarrToMat(*it);
        sprintf(filename, (MODEL_PATH+"/"+to_string(vect_index)+".jpg").c_str());
        cout<<filename<<endl;
        imwrite(filename, src);
    }
    cout <<"模板生成成功！" << endl;
    return 0;
}

int match(string btn_r,int symmetry){
    char key;
    Mat frame,img;
    string match_num;

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
	string data[3];
	fileinput >> data[0];
	fileinput >> data[1];
	fileinput >> data[2];
	fileinput.close();

	//内接矩的坐标
//	CvRect rect_inner, rect_outter;
//	string matchRectValue = exchange(data[0], ":")[1];
//	vector<string> matchs = exchange(matchRectValue, ",");
//	rect_inner.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
//	rect_inner.y = atoi(matchs[1].c_str());
//	rect_inner.width = atoi(matchs[2].c_str());
//	rect_inner.height = atoi(matchs[3].c_str());

    //内接圆半径
    int r=0;
    string matchRectValue = exchange(data[0], ":")[1];
    vector<string> matchs = exchange(matchRectValue, ",");
    r = atoi(matchs[2].c_str());
    cout<<"内圆半径："<<r<<endl;

//    cout<<rect_inner.x<<" "<<rect_inner.y<<" "<<rect_inner.width<<" "<<rect_inner.height<<endl;
    //外接矩的坐标
    CvRect rect_outter;
    matchRectValue = exchange(data[1], ":")[1];
    matchs = exchange(matchRectValue, ",");
    rect_outter.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
    rect_outter.y = atoi(matchs[1].c_str());
    rect_outter.width = atoi(matchs[2].c_str());
    rect_outter.height = atoi(matchs[3].c_str());

    cout<<rect_outter.x<<" "<<rect_outter.y<<" "<<rect_outter.width<<" "<<rect_outter.height<<endl;

    //外接圆的坐标
    Point center;
    int radius; //外接圆半径
    matchRectValue = exchange(data[2], ":")[1];
    matchs = exchange(matchRectValue, ",");
    center.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
    center.y = atoi(matchs[1].c_str());
    radius = atoi(matchs[2].c_str());

    cout<<center.x<<" "<<center.y<<" "<<radius<<endl;

    //读入图片到容器中
	Mat modelMat;
	unsigned int index = 0;
	unsigned int total = 4;
	char filename[100];
	vector<Mat> modelMats;
	IplImage* matchImage; /*= cvCreateImage(cvSize(rect_outter.width, rect_outter.height), IPL_DEPTH_8U, 1);*/
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

    IplImage* matchRectImage, *matchRectImage2;
    IplImage matchBlurImage;
    Mat blurMatchDstMat,dstMatchRect,dstMatchRect2/*,frame*/;
    int direction = 1;//旋转方向 0-左  1-右
    int nbyte = 0;
    int test_no = 1;//测试图编号

    float value = 0,value2=0;
    float max = 0;
    int res = 0;
    int iter = 0;
    cv::Point matchLoc;
    //逐一匹配
//            double Time = (double)cvGetTickCount();
    clock_t start,finish;
    double total_time;

    ofstream outFile;
    outFile.open(TEST_PATH+btn_r+'/'+btn_r+"_data.csv", ios::out); // 打开模式可省略

    frame = imread((TEST_PATH+btn_r+'/'+to_string(test_no)+".jpg").c_str(), 1);
    Mat img_dark, diff;
    vector<Mat> rotate_imgs;
    vector<float> values;//11张图的4个角度匹配值
    map<string,float> angle_max_value{{"0",0},{"90",0},{"180",0},{"270",0}};//每个 角度最大值
    Mat mask = Mat::zeros(frame.size(), CV_8UC1);
//    circle(mask, center, radius, Scalar(255), -1, 8); //外接圆mask
    namedWindow("frame",0);
    resizeWindow("frame",width/2,width/2);
    while(1){
        direction=1;
        key = waitKey(10);
        imshow("frame", frame);

        if(test_no<=80)
        {
            cout<<test_no<<endl;
            frame = imread((TEST_PATH+btn_r+'/'+to_string(test_no++)+".jpg").c_str(), 1);
            img = frame.clone();
            circle(frame,center,r,Scalar(0,0,255),2,1,0);
            circle(frame,center,radius,Scalar(0,0,255),2,1,0);
//            rectangle(frame,rect_inner,Scalar(0,0,255),2,1,0);
//            rectangle(frame,rect_outter,Scalar(0,0,255),2,1,0);
            imshow("frame", frame);
            blurMatchDstMat = pertImage0(img);

            img_dark = Mat::zeros(blurMatchDstMat.size(), CV_8UC1);
            circle(mask, center, radius, Scalar(255), -1, 8); //外接圆mask
            blurMatchDstMat.copyTo(img_dark, mask);

            blurMatchDstMat=img_dark.clone();
            img_dark.setTo(Scalar(0));
            mask.setTo(Scalar(0));
            circle(mask, center, r, Scalar(255), -1, 8);
            img_dark.copyTo(blurMatchDstMat,mask);


//            imshow("src",blurMatchDstMat);
//            Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
//            erode(blurMatchDstMat,blurMatchDstMat,element);//腐蚀
//            dilate(blurMatchDstMat,blurMatchDstMat,element);//膨胀
//            morphologyEx(blurMatchDstMat, blurMatchDstMat, MORPH_OPEN, element);  //开运算
//            morphologyEx(blurMatchDstMat, blurMatchDstMat, MORPH_CLOSE, element);  //闭运算
//            morphologyEx(blurMatchDstMat, blurMatchDstMat, MORPH_GRADIENT, element);  //形态学梯度运算
//            morphologyEx(blurMatchDstMat, blurMatchDstMat, MORPH_TOPHAT, element);  //顶帽运算
//            morphologyEx(blurMatchDstMat, blurMatchDstMat, MORPH_BLACKHAT, element);  //黒帽运算
//            imshow("mor",blurMatchDstMat);
//            cvWaitKey(0);



            rotate_imgs = rotation(blurMatchDstMat,rect_outter,center); //旋转后11张图像
//            cout<<"rotate_imgs大小："<<rotate_imgs.size()<<endl;

//            blurMatchDstMat(rect_inner).setTo(0);
//            matchBlurImage = (IplImage)blurMatchDstMat;

            //截匹配区域
            //Mat roi2 = img(Rect(10,10,100,100));
//            matchRectImage = getRectImage(&matchBlurImage, rect_outter.x, rect_outter.y, rect_outter.width, rect_outter.height);
//            dstMatchRect = cvarrToMat(matchRectImage);
//            imshow("blur", blurMatchDstMat);

            value = 0;
            max = 0;
            res = 0;
            iter = 0;
            start = clock();
            for (vector <Mat>::iterator it = modelMats.begin(); it != modelMats.end(); it++){ //四张模板图
                for(vector <Mat>::iterator rotate=rotate_imgs.begin(); rotate!=rotate_imgs.end(); rotate++){//11张旋转图
                    imshow("src", *rotate);
                    cvWaitKey(2);
                    value = temp_match(*it, *rotate, matchLoc, cv::TM_CCORR_NORMED);
                    values.push_back(value);//11个值一组
                    printf("this is %d angle,value = %f\n ", iter, value);
                    absdiff(*rotate,*it,diff);
                    imshow("diff",diff);
//                    cvWaitKey(0);

                }
                angle_max_value[to_string(iter)]=*max_element(values.begin(), values.end());
                printf("this is %d angle,最大value = %f\n ", iter, *max_element(values.begin(), values.end()));
                values.clear();
                iter+=90;
            }
            for(map<string, float>::iterator angle = angle_max_value.begin(); angle != angle_max_value.end(); angle++) {
                if (angle->second > max){
                    max=angle->second;
                    res = atoi((angle->first).c_str());
                }
            }
            finish = clock();
            total_time = (double)((finish-start)*1000/CLOCKS_PER_SEC);//ms
//            printf("match run time = %gms\n", total_time);//毫秒
//            if (angle==270){
//                angle=90;
//                direction = 0;
//            }
            if(res-((test_no-2)%4)*90==0 || (abs(res-((test_no-2)%4)*90)==180 && symmetry))
                outFile <<to_string(test_no-1)+".jpg"<< ',' << res << endl;
            else
                outFile <<to_string(test_no-1)+".jpg"<< ',' << res<<" error" << endl;
            max>=0.80 ? (direction==0? printf("旋转角度 = %d ,方向 左\n\n", res):printf("旋转角度 = %d ,方向 右\n\n", res)) : printf("无法识别\n");
            rotate_imgs.clear();//清空vector
        }
        else if(test_no>=80 || nbyte==4 && DATA_REC[1]==0xFE)//退出识别
        {
            outFile.close();
            destroyAllWindows();
            return 0;
        }// else{outFile.close();return 0;}
    }
}



int main()
{
//    Mat image=imread("20180706/model/0.jpg");
//    int dx=image.cols/2;   //计算图像中心
//    int dy=image.rows/2;
//    double pi=3.141592653589793;
//    double angle=pi/12;  //旋转角度
//    float map[6];
//    Mat map_matrix;
//    map_matrix=getRotationMatrix2D(Point(dx,dy),-5,1.0);  //旋转中心，角度，缩放比例
//    Mat src(image.rows,image.cols,CV_8UC3);
//    warpAffine(image,src,map_matrix,Size(image.cols,image.rows));
//    namedWindow("src");
//    imshow("src",src);
//    waitKey(0);

    int symmetry;
    string r;
    cout<<"请输入纽扣半径和是否对称（0-1）：";
    cin>>r>>symmetry;
//    cin>>symmetry;
//    cout<<r<<" d"<<symmetry;
    generate_temp(r);
    match(r,symmetry);
    return 0  ;
}

