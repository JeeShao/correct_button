#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include<opencv2/opencv.hpp>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include<iostream>
#include<string>
#include<vector>
#include "rect_image.h"
#include "match.h"
using namespace std;
using namespace cv;
//ofstream out("model.txt", ios::app);

const string FILE_PATH = "../20180706/";
const string MODEL_PATH = FILE_PATH+"model/";


//图像预处理,灰度,滤波
//Mat pertImage(IplImage* srcImage){
//    Mat srcMat = cvarrToMat(srcImage);
//    Mat grayImage;
//    cvtColor(srcMat, grayImage, CV_BGR2GRAY);
//    Mat blurImage;
//    GaussianBlur(grayImage, blurImage, Size(3, 3), 0, 0);
//    return blurImage;
//}

//截取图像
//IplImage* getRectImage(IplImage *srcImage, int x, int y, int width, int height){
//    cvSetImageROI(srcImage, cvRect(x, y, width, height));
//    IplImage *rectImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
//    cvCopy(srcImage, rectImage, NULL);
//    return rectImage;
//}


int generate_temp(){
    int num = 0;  //当前最大模板编号
    DIR* dir = opendir(MODEL_PATH.c_str());//打开指定目录
    dirent* p = NULL;//定义遍历指针
    while((p = readdir(dir)) != NULL)//开始逐个遍历
    {
        //过滤掉目录中"."和".."隐藏文件
        if(p->d_name[0] != '.')//d_name是一个char数组，存放当前遍历到的文件名
        {
            string name = string(p->d_name);
            num = atoi(name.c_str())>num ? atoi(name.c_str()) : num;
            //cout<<name<<endl;
        }
    }
    closedir(dir);



    //截取纽扣的坐标
//    const int square_X = 160;
//    const int square_Y = 115;
//    const int square_Width = 274;
//    const int square_Height = 241;
//    file << "截取纽扣的参数:" << square_X << "," << square_Y << "," << square_Width << "," << square_Height << endl;
    //截取匹配区域的坐标-2018423
    /*const int match_X = 100;
    const int match_Y = 200;
    const int match_Width = 30;
    const int match_Height = 30;*/
    //截取匹配区域的坐标-2018620
//    const int match_X = 122;
//    const int match_Y = 197;
//    const int match_Width = 35;
//    const int match_Height = 30;
    //待匹配区域坐标
//    const int waitMatch_X = 128;
//    const int waitMatch_Y = 197;
//    const int waitMatch_Width = 27;
//    const int waitMatch_Height = 27;
//    file << "截取待匹配区域的参数:" << waitMatch_X << "," << waitMatch_Y << "," << waitMatch_Width << "," << waitMatch_Height << endl;
//    file.close();

    //*********************××××××××××××××××
    //从摄像头读取360度模板原图.保存至20180706/
    //*********************××××××××××××××××

    //截取匹配区域
    IplImage* org_img = cvLoadImage((FILE_PATH+"0.jpg").c_str(),1);
    int *res_pixs;
    res_pixs = rectImage(org_img);
    //截取匹配区域的坐标-2018620
    const int match_X = res_pixs[0];
    const int match_Y = res_pixs[1];
    const int match_Width = res_pixs[2];
    const int match_Height = res_pixs[3];

    string a  = MODEL_PATH+to_string(num+1);
    const char *path = a.c_str();
    int isCreate = mkdir(path,S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);//为新模板创建文件夹
    if( !isCreate )
        printf("create path succeed!");
    else
        printf("create path failed!");

    //先清空文件
    fstream file;
    file.open(a+"/model.txt", ios::out);
    if (!file.is_open()) // 检查文件是否成功打开
    {
        cout <<num+1<< "号模板文件打开失败!" << endl;
        return 0;
    }

    file.clear();

    //待匹配区域为手选区域下半部分的中间区域
    file << "待匹配区域的参数:" << match_X+match_Width/4 << "," << match_Y+match_Height/2 << "," << match_Width/2 << "," << match_Height/2 << endl;
    file << "匹配区域的参数:" << match_X << "," << match_Y << "," << match_Width << "," << match_Height << endl;
    file.close();
//    cvWaitKey(0);
    //获取原图像
    unsigned int index = 0;
    unsigned int total = 360;
    char filename[100];
    //放模板匹配区域的图像
    vector<IplImage*> modelImages;
    //原图像
    IplImage* srcImage = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
    //读取文件夹中的图像
    for (index; index < total; index++){
        sprintf(filename, (FILE_PATH+to_string(index)+".jpg").c_str());
        srcImage = cvLoadImage(filename, 1);
        if (srcImage != NULL){
            //灰度处理
            Mat blurModelDstMat = pertImage(srcImage);
            //cvCvtColor(srcImage, grayImage, CV_RGB2GRAY);
            IplImage* blurImage = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
            IplImage temp_1 = (IplImage)blurModelDstMat;
            blurImage = &temp_1;
            //截取大图
//            IplImage* squareImage = cvCreateImage(cvSize(square_Width, square_Height), IPL_DEPTH_8U, 1);
//            squareImage = getRectImage(blurImage, square_X, square_Y, square_Width, square_Height);
//            cvResetImageROI(squareImage);
            //截取匹配区域
            IplImage* matchImage = cvCreateImage(cvSize(match_Width, match_Height), IPL_DEPTH_8U, 1);
            matchImage = getRectImage(blurImage, match_X, match_Y, match_Width, match_Height);
            //存入容器中
            modelImages.push_back(matchImage);
            //index = index + 5;
        }
        else{
            cout << "模板图像少于360张！" << endl;
            break;
        }
    }
    //取出匹配区域进行二值化并保存，其实可以不用存入容器中的，直接在上面二值化保存就行
    unsigned int vect_index = 0;
    for (vector<IplImage*>::iterator it = modelImages.begin(); it != modelImages.end(); it++)
    {
        Mat src = cvarrToMat(*it);
        //Mat dst;
        //对匹配区域二值化
        //threshold(src, dst, 0, 255, CV_THRESH_OTSU);
        sprintf(filename, (a+"/"+to_string(vect_index)+".jpg").c_str());
        cout<<filename<<endl;
        //imshow(filename, dst);
        imwrite(filename, src);
        vect_index = vect_index + 1;
    }
    cout << (num+1)<<"号模板生成成功！" << endl;
    getchar();
    cvWaitKey(0);
    return 0;
}

int main()
{
    int key;
    cout<<"输入功能编号(1-生成模板 2-角度检测)：";
    cin>>key;
    switch (key){
        case 1: generate_temp();break;
        case 2: match();break;
        default:return 0;
        }
    return 1;
}

