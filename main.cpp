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
#include "rect_image.h"
#include "match.h"
#include "serial.h"
using namespace std;
using namespace cv;

const string FILE_PATH = "20180706/";
const string MODEL_PATH = FILE_PATH+"model/";
const string ORG_PATH = FILE_PATH+"org_imgs/";

TTY_INFO *ptty;  //串口
unsigned char DATA_MSG[] = {0xDE,0xA9,00,0xFF,0xFF};  //通知
unsigned char DATA_ANGLE[] = {0xDE,0xA1,01,00,0xFF,0xFF}; //旋转角度
unsigned char DATA_REC[8] = {}; //接受串口信息

//VideoCapture  capture;

int generate_temp(){
    int num = 0,cur_no=0,sum=4;  //num当前最大模板编号  sum采集模板图片数（4） cur_no当前采集模板编号
    char key;
    DIR* dir = opendir(MODEL_PATH.c_str());//打开指定目录
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

    //*********************××××××××××××××××
    //从摄像头读取4个角度模板原图.保存至20180706/
    Mat frame;
//    if(!capture.isOpened()) {
//        capture.open(0);
//    }
//    if(!capture.isOpened()){
//        cerr<<"摄像头打开失败！"<<endl;
//        return 0;
//    }
    namedWindow("frame",1);
//    DATA_MSG[2] = 03;  //完成
//    sendnTTY(ptty,DATA_MSG,5);//准备好
    while(0){
        key = waitKey(10);
//        capture>>frame;
//        imshow("frame",frame);
        if(key == 27 || cur_no>=sum)
            break;//按ESC键退出程序
        if(recvnTTY(ptty,DATA_REC,4)==4)//拍照
        {
//            cout<<"rec"<<endl;
            printf("0x%02x",DATA_REC[1]);
            if(DATA_REC[1]==0xFD)//拍照
            {
                if(sendnTTY(ptty,DATA_MSG,5)==5){ //完成
                    cout<<cur_no<<endl;
                    imwrite((ORG_PATH+to_string(cur_no++)+".jpg").c_str(),frame);//图片保存到本工程目录中
                }else
                    cout<<"send code error";
            }else if(DATA_REC[1]==0xFE)  //退出模板
                break;
            memset(DATA_REC,0,8);
        }
    }
    destroyWindow("frame");

    //截取匹配区域
    IplImage* org_img = cvLoadImage((ORG_PATH+"0.jpg").c_str(),1);
    int *res_pixs ,*res_pixs2;
    res_pixs = rectImage(org_img);
    //截取匹配区域的坐标-2018620
    const int match_X = res_pixs[0];
    const int match_Y = res_pixs[1];
    const int match_Width = res_pixs[2];
    const int match_Height = res_pixs[3];

    res_pixs2 = rectImage(org_img);
    const int match_X2 = res_pixs2[0];
    const int match_Y2 = res_pixs2[1];
    const int match_Width2 = res_pixs2[2];
    const int match_Height2 = res_pixs2[3];

    string a  = MODEL_PATH+to_string(num+1);
    const char *path = a.c_str();
    int isCreate = mkdir(path,S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);//为新模板创建文件夹
    if( !isCreate )
        printf("create path succeed!\n");
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

    //待匹配区域为手选区域zhongjian部分的中间区域
    file << "待匹配区域的参数1:" << match_X+match_Width/4 << "," << match_Y+match_Height/4 << "," << match_Width/2 << "," << match_Height/2 << endl;
    file << "待匹配区域的参数2:" << match_X2+match_Width2/4 << "," << match_Y2+match_Height2/4 << "," << match_Width2/2 << "," << match_Height2/2 << endl;
    file.close();
    //获取原图像
    unsigned int index = 0;
    unsigned int total = 4;
    char filename[100];
    //放模板匹配区域的图像
    vector<IplImage*> modelImages;
    //原图像
    IplImage* srcImage = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
    //读取文件夹中的图像
    for (index; index < total; index++){
        sprintf(filename, (ORG_PATH+to_string(index)+".jpg").c_str());
        srcImage = cvLoadImage(filename, 1);
        if (srcImage != NULL){
            //灰度处理
            Mat blurModelDstMat = pertImage(srcImage);
            IplImage* blurImage ;
            IplImage temp_1 = (IplImage)blurModelDstMat;
            blurImage = &temp_1;

            IplImage* matchImage ;
            matchImage = getRectImage(blurImage, match_X, match_Y, match_Width, match_Height);
            //存入容器中
            modelImages.push_back(matchImage);
            matchImage = getRectImage(blurImage, match_X2, match_Y2, match_Width2, match_Height2);
            //存入容器中
            modelImages.push_back(matchImage);
        }
        else{
            cout << "模板图像少于4张！" << endl;
            break;
        }
    }
    //取出匹配区域进行二值化并保存，其实可以不用存入容器中的，直接在上面二值化保存就行
    unsigned int vect_index = 0;
    for (vector<IplImage*>::iterator it = modelImages.begin(); it != modelImages.end(); it+=2)
    {
        Mat src = cvarrToMat(*it);
        sprintf(filename, (a+"/"+to_string(vect_index)+'_'+'0'+".jpg").c_str());
        cout<<filename<<endl;
        imwrite(filename, src);

        src = cvarrToMat(*(it+1));
        sprintf(filename, (a+"/"+to_string(vect_index)+'_'+'1'+".jpg").c_str());
        cout<<filename<<endl;
        imwrite(filename, src);

        vect_index = vect_index + 1;
    }

    //删除模板图像文件
//    DIR* org_dir = opendir(ORG_PATH.c_str());//打开指定目录
//    dirent* p1 = NULL;//定义遍历指针
//    while((p1 = readdir(org_dir)) != NULL)//开始逐个遍历
//    {
//        //过滤掉目录中"."和".."隐藏文件
//        if(p1->d_name[0] != '.')//d_name是一个char数组，存放当前遍历到的文件名
//        {
//            string name = string(p1->d_name);
//            remove((ORG_PATH+name).c_str());
//        }
//    }
//    closedir(org_dir);

    cout << (num+1)<<"号模板生成成功！" << endl;
    getchar();
    cvWaitKey(0);
    return 0;
}

int match(){

	int model_num;
    string match_num;
	cout<<"输入需要匹配模板编号：";
	cin>>model_num;


//    DATA_MSG[2] = 03;  //完成
//    sendnTTY(ptty,DATA_MSG,5);//准备好
	//先读入模板model.txt
	ifstream fileinput;
    try {
//        cout<<MODEL_PATH+to_string(model_num)+"/model.txt";
        fileinput.open(MODEL_PATH+to_string(model_num)+"/model.txt");
    }catch ( exception &e){
        cerr << "Caught: " << e.what( ) << endl;
        cerr << "Type: " << typeid( e ).name( ) << endl << endl;
    }

    if (!fileinput.is_open())
    {
        cout<<model_num<<"号模板不存在或打开模板文件失败！"<<endl;
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

    matchRectValue = exchange(data[1], ":")[1];
    matchs = exchange(matchRectValue, ",");
    matchRect2.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
    matchRect2.y = atoi(matchs[1].c_str());
    matchRect2.width = atoi(matchs[2].c_str());
    matchRect2.height = atoi(matchs[3].c_str());
	//读入图片到容器中
	Mat modelMat;
	unsigned int index = 0;
	unsigned int total = 4;
	char filename[100];
	vector<Mat> modelMats;
	IplImage* matchImage = cvCreateImage(cvSize(matchRect.width, matchRect.height), IPL_DEPTH_8U, 1);
	for (index; index < total; index++){
        for(int no=0; no<2; no++){
            sprintf(filename, (MODEL_PATH+to_string(model_num)+"/"+to_string(index)+'_'+to_string(no)+".jpg").c_str());
            matchImage = cvLoadImage(filename, 1);
            if (matchImage != NULL){
                Mat modelMat = cvarrToMat(matchImage);
                modelMats.push_back(modelMat);
            }
            else{
                cout<<model_num<< "号模板图像不足4张！" << endl;
                break;
            }
        }
	}

    Mat matchSrcImage,blurMatchDstMat,dstMatchRect,dstMatchRect2;
    IplImage matchBlurImage;
    IplImage* matchRectImage, *matchRectImage2;
    cout<<endl;
    cout<<"输入匹配图No：";
    cin>>match_num;
    cout<<(string("test0914/")+match_num+".jpg").c_str()<<endl;
    matchSrcImage = imread((string("test0914/")+match_num+".jpg").c_str(),1);
    if(!matchSrcImage.data){
        cout<<"匹配图No ERROR"<<endl;
    }

    blurMatchDstMat = pertImage0(matchSrcImage);
    matchBlurImage = (IplImage)blurMatchDstMat;//

    //截匹配区域
    matchRectImage = getRectImage(&matchBlurImage, matchRect.x, matchRect.y, matchRect.width, matchRect.height);
    matchRectImage2 = getRectImage(&matchBlurImage, matchRect2.x, matchRect2.y, matchRect2.width, matchRect2.height);
    dstMatchRect = cvarrToMat(matchRectImage);
    dstMatchRect2 = cvarrToMat(matchRectImage2);

    imwrite("../match_img.jpg", dstMatchRect);

    float value = 0,value2=0;
    double max = 0;
    int angle = 0;
    int iter = 0;
    cv::Point matchLoc;
    //逐一匹配
    unsigned int vect_index = 0;
    double Time = (double)cvGetTickCount();
    for (vector <Mat>::iterator it = modelMats.begin(); it != modelMats.end(); it+=2){
        cvtColor(*it, *it, CV_BGR2GRAY);
        cvtColor(*(it+1), *(it+1), CV_BGR2GRAY);
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
    Time = (double)cvGetTickCount() - Time;
    printf("match run time = %gms\n", 36 * Time / (cvGetTickFrequency() * 1000));//毫秒
    max>=0.94 ? printf("正确旋转角度 = %d\n\n ", angle) : printf("无法识别\n");
//    }




    while(0){
        int direction = 1;//旋转方向 0-左  1-右
        int nbyte = 0;
//        DATA_REC = {};  //清空
        nbyte = recvnTTY(ptty,DATA_REC,4);

        if(nbyte==4 && DATA_REC[1]==0xFD)  //拍照
        {
            //待测图像,这里换成摄像头代码
            IplImage matchSrcImage;
            Mat frame;
//            capture>>frame;
//            imshow("a",frame);
//            imwrite("test_img.jpg", frame);
            Mat blurMatchDstMat = pertImage0(frame);
            IplImage matchBlurImage = (IplImage)blurMatchDstMat;//
            //截匹配区域
            IplImage* matchRectImage = getRectImage(&matchBlurImage, matchRect.x, matchRect.y, matchRect.width, matchRect.height);
            Mat dstMatchRect = cvarrToMat(matchRectImage);

            imwrite("../match_img.jpg", dstMatchRect);

            float value = 0;
            double max = 0;
            int angle = 0;
            int iter = 0;
            cv::Point matchLoc;
            //逐一匹配
            unsigned int vect_index = 0;
            double Time = (double)cvGetTickCount();
            for (vector < Mat >::iterator it = modelMats.begin(); it != modelMats.end(); it++){
                cvtColor(*it, *it, CV_BGR2GRAY);
                value = temp_match(*it, dstMatchRect, matchLoc, cv::TM_CCORR_NORMED);
                //value = GetSim(dstMatchRect, *it);
                printf("this is %d angle,value = %f\n ", iter, value);
                if (max<value){
                    max = value;
                    angle = iter;
                }
                iter+=90;
            }
            Time = (double)cvGetTickCount() - Time;
            printf("match run time = %gms\n", 36 * Time / (cvGetTickFrequency() * 1000));//毫秒
            max>=0.99 ? printf("正确旋转角度 = %d\n ", angle) : printf("无法识别\n");

        }
        else if(nbyte==4 && DATA_REC[1]==0xFE)//退出识别
        {
            return 0;
        }

        if(cvWaitKey(10)==27)  //ESC退出
        {
            return 0;
        }
    }
}

int main()
{
    int key;

    while(1)
    {
        cout<<"\n"<<"输入功能编号(1-生成模板 2-角度检测)：";
        cin>>key;
        switch (key){
            case 1: generate_temp();break;
            case 2: match();break;
//            case 'q':capture.release(); return 1;
            case 'q': return 1;
//            default:capture.release();return 0;
            default:return 0;
        }
    }
}

