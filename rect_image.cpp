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

//光照矫正
Mat illumination(Mat bgr_image){
    // READ RGB color image and convert it to Lab
//    cv::Mat bgr_image = cv::imread("20180706/org_imgs/1.jpg");
    cv::Mat lab_image;
    cv::cvtColor(bgr_image, lab_image, CV_BGR2Lab);

    // Extract the L channel
    std::vector<cv::Mat> lab_planes(3);
    cv::split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

    // apply the CLAHE algorithm to the L channel
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(4);
    cv::Mat dst;
    clahe->apply(lab_planes[0], dst);

    // Merge the the color planes back into an Lab image
    dst.copyTo(lab_planes[0]);
    cv::merge(lab_planes, lab_image);

    // convert back to RGB
    cv::Mat image_clahe;
    cv::cvtColor(lab_image, image_clahe, CV_Lab2BGR);
    return image_clahe;
}

//白平衡
Mat whiteBalance(Mat g_srcImage){
    Mat dstImage;
    vector<Mat> g_vChannels;

    //分离通道
    split(g_srcImage,g_vChannels);
    Mat imageBlueChannel = g_vChannels.at(0);
    Mat imageGreenChannel = g_vChannels.at(1);
    Mat imageRedChannel = g_vChannels.at(2);

    double imageBlueChannelAvg=0;
    double imageGreenChannelAvg=0;
    double imageRedChannelAvg=0;

    //求各通道的平均值
    imageBlueChannelAvg = mean(imageBlueChannel)[0];
    imageGreenChannelAvg = mean(imageGreenChannel)[0];
    imageRedChannelAvg = mean(imageRedChannel)[0];

    //求出个通道所占增益
    double K = (imageRedChannelAvg+imageGreenChannelAvg+imageRedChannelAvg)/3;
    double Kb = K/imageBlueChannelAvg;
    double Kg = K/imageGreenChannelAvg;
    double Kr = K/imageRedChannelAvg;

    //更新白平衡后的各通道BGR值
    addWeighted(imageBlueChannel,Kb,0,0,0,imageBlueChannel);
    addWeighted(imageGreenChannel,Kg,0,0,0,imageGreenChannel);
    addWeighted(imageRedChannel,Kr,0,0,0,imageRedChannel);

    merge(g_vChannels,dstImage);//图像各通道合并
    return dstImage;
}

void unevenLightCompensate0(Mat &image, int blockSize)
{
    if (image.channels() == 3) cvtColor(image, image, 7);
    double average = mean(image)[0];
    int rows_new = ceil(double(image.rows) / double(blockSize));
    int cols_new = ceil(double(image.cols) / double(blockSize));
    Mat blockImage;
    blockImage = Mat::zeros(rows_new, cols_new, CV_32FC1);
    for (int i = 0; i < rows_new; i++)
    {
        for (int j = 0; j < cols_new; j++)
        {
            int rowmin = i*blockSize;
            int rowmax = (i + 1)*blockSize;
            if (rowmax > image.rows) rowmax = image.rows;
            int colmin = j*blockSize;
            int colmax = (j + 1)*blockSize;
            if (colmax > image.cols) colmax = image.cols;
            Mat imageROI = image(Range(rowmin, rowmax), Range(colmin, colmax));
            double temaver = mean(imageROI)[0];
            blockImage.at<float>(i, j) = temaver;
        }
    }
    blockImage = blockImage - average;
    Mat blockImage2;
    resize(blockImage, blockImage2, image.size(), (0, 0), (0, 0), INTER_CUBIC);
    Mat image2;
    image.convertTo(image2, CV_32FC1);
    Mat dst = image2 - blockImage2;
    dst.convertTo(image, CV_8UC1);
}


//图像预处理,灰度，滤波
Mat pertImage(Mat srcImage){
    Mat grayImage;
    Mat blurImage;
    if(srcImage.channels()==3) {
        srcImage = whiteBalance(srcImage);
//	unevenLightCompensate(srcImage,12);

        cvtColor(srcImage, grayImage, CV_BGR2GRAY);
    } else
        grayImage = srcImage.clone();

//    cv::normalize(grayImage, grayImage, 0,1, cv::NORM_L2);
//	convertScaleAbs(grayImage,grayImage);
//	Mat gainMat(grayImage.rows, grayImage.cols, CV_8UC1, Scalar::all(255));

//	equalizeHist(grayImage,grayImage);

    const int maxVal = 255;
    int blockSize = 3;	//取值3、5、7....等
    int constValue = 5;
    int adaptiveMethod = 0;
    int thresholdType = 1;
    /*
        自适应阈值算法
        0:ADAPTIVE_THRESH_MEAN_C
        1:ADAPTIVE_THRESH_GAUSSIAN_C
        --------------------------------------
        阈值类型
        0:THRESH_BINARY
        1:THRESH_BINARY_INV
    */
    //---------------【4】图像自适应阈值操作-------------------------
//    adaptiveThreshold(grayImage, grayImage, maxVal, adaptiveMethod, thresholdType, blockSize, constValue);



    GaussianBlur(grayImage, blurImage, Size(3, 3), 0, 0);
    return blurImage;
}

int* rectImage(Mat srcImage){

//    IplImage* srcImage = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
//    srcImage = cvLoadImage("../test.jpg", 1);
    Mat blurModelDstMat = pertImage(srcImage);
//    IplImage* blurImage = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
    IplImage temp_1 = (IplImage)blurModelDstMat;
//    blurImage = &temp_1;

    //如果是要确定大圆的坐标，用这行代码！！！之后记得不需要的时候将这个图片保存！重命名！
    org = &temp_1;
    //如果是确定原图像已经截取出的大圆图像上的匹配区域，用这行代码！！！
//	org = cvLoadImage("../word.jpg",1);
    //截取之后二值化
    img = cvCloneImage(org);
    tmp = cvCloneImage(org);
    namedWindow("img", 0);
    resizeWindow("img",tmp->width/2,tmp->height/2);
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