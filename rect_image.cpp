#include "rect_image.h"

using namespace std;
using namespace cv;
//ofstream out("model.txt", ios::app);

IplImage *img = 0;
IplImage *tmp = 0;
IplImage *org = 0;
IplImage* dst = 0;
int match_pixs[4];
//bool key = false;//true => 完成截图
//鼠标截取字母模板
void on_mouse(int event, int x, int y, int flags, void* ustc)
{
    static CvPoint pre_pt = { -1, -1 };
    static CvPoint cur_pt = { -1, -1 };
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);
    char temp[16];

    if (event == CV_EVENT_LBUTTONDOWN)
    {
        cvCopy(org, img);
        sprintf(temp, "(%d,%d)", x, y);
        pre_pt = cvPoint(x, y);
        cvPutText(img, temp, pre_pt, &font, cvScalar(0, 0, 0, 255));
        cvCircle(img, pre_pt, 3, cvScalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);
        cvShowImage("img", img);
        cvCopy(img, tmp);
    }
    else if (event == CV_EVENT_MOUSEMOVE && !(flags & CV_EVENT_FLAG_LBUTTON))
    {
        cvCopy(tmp, img);
        sprintf(temp, "(%d,%d)", x, y);
        cur_pt = cvPoint(x, y);
        cvPutText(img, temp, cur_pt, &font, cvScalar(0, 0, 0, 255));
        cvShowImage("img", img);
    }
    else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))
    {
        cvCopy(tmp, img);
        sprintf(temp, "(%d,%d)", x, y);
        cur_pt = cvPoint(x, y);
        cvPutText(img, temp, cur_pt, &font, cvScalar(0, 0, 0, 255));
        cvRectangle(img, pre_pt, cur_pt, cvScalar(0, 255, 0, 0), 1, 8, 0);
        cvShowImage("img", img);
    }
    else if (event == CV_EVENT_LBUTTONUP)
    {
        cvCopy(tmp, img);
        sprintf(temp, "(%d,%d)", x, y);
        cur_pt = cvPoint(x, y);
        cvPutText(img, temp, cur_pt, &font, cvScalar(0, 0, 0, 255));
        cvCircle(img, cur_pt, 3, cvScalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);
        cvRectangle(img, pre_pt, cur_pt, cvScalar(0, 255, 0, 0), 1, 8, 0);
        cvShowImage("img", img);

        cvCopy(img, tmp);
        int width = abs(pre_pt.x - cur_pt.x);
        int height = abs(pre_pt.y - cur_pt.y);
        if (width == 0 || height == 0)
        {
            cvDestroyWindow("dst");
            return;
        }
        dst = cvCreateImage(cvSize(width, height), org->depth, org->nChannels);
        CvRect rect;
        if (pre_pt.x<cur_pt.x && pre_pt.y<cur_pt.y)
        {
            rect = cvRect(pre_pt.x, pre_pt.y, width, height);
        }
        else if (pre_pt.x>cur_pt.x && pre_pt.y<cur_pt.y)
        {
            rect = cvRect(cur_pt.x, pre_pt.y, width, height);
        }
        else if (pre_pt.x>cur_pt.x && pre_pt.y>cur_pt.y)
        {
            rect = cvRect(cur_pt.x, cur_pt.y, width, height);
        }
        else if (pre_pt.x<cur_pt.x && pre_pt.y>cur_pt.y)
        {
            rect = cvRect(pre_pt.x, cur_pt.y, width, height);
        }
        cvSetImageROI(org, rect);
        cvCopy(org, dst);
        cvResetImageROI(org);
        cvNamedWindow("dst", 1);
        cvShowImage("dst", dst);
        cout << "截取区域的参数:" << rect.x << "," << rect.y << "," << rect.width << "," << rect.height << endl;
        match_pixs[0] = rect.x;
        match_pixs[1] = rect.y;
        match_pixs[2] = rect.width;
        match_pixs[3] = rect.height;
        /*const char *pstrWindowsCircleName = "dst";
        cvNamedWindow(pstrWindowsCircleName, CV_WINDOW_AUTOSIZE);
        cvShowImage(pstrWindowsCircleName, dst);
        cvSaveImage("截取之后的图片.jpg", dst);*/
        //cvWaitKey(0);
        //cvDestroyAllWindows();

    }
    //双击保存
    else if (event == CV_EVENT_LBUTTONDBLCLK)
    {
//        cvSaveImage("../matching_area.jpg", dst);
        destroyAllWindows();
//        key = true;

    }
}

//图像预处理,灰度，滤波
Mat pertImage(IplImage* srcImage){
    Mat srcMat = cvarrToMat(srcImage);
    Mat grayImage;
    cvtColor(srcMat, grayImage, CV_BGR2GRAY);
    Mat blurImage;
    GaussianBlur(grayImage, blurImage, Size(3, 3), 0, 0);
    return blurImage;
}

int* rectImage(IplImage *srcImage){

//    IplImage* srcImage = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
//    srcImage = cvLoadImage("../test.jpg", 1);
    Mat blurModelDstMat = pertImage(srcImage);
    IplImage* blurImage = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
    IplImage temp_1 = (IplImage)blurModelDstMat;
    blurImage = &temp_1;

    //如果是要确定大圆的坐标，用这行代码！！！之后记得不需要的时候将这个图片保存！重命名！
    org = blurImage;
    //如果是确定原图像已经截取出的大圆图像上的匹配区域，用这行代码！！！
//	org = cvLoadImage("../word.jpg",1);
    //截取之后二值化
    img = cvCloneImage(org);
    tmp = cvCloneImage(org);
    cvNamedWindow("img", CV_WINDOW_AUTOSIZE);
    cvvShowImage("img",tmp);
    cvSetMouseCallback("img", on_mouse, 0);
    cvWaitKey(0);

//    while (!key && cvWaitKey(0))
//    {
//        cout<<"aa"<<endl;
//    }
//    key = false;
    return match_pixs;

}