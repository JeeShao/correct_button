#include "match.h"

const string FILE_PATH = "../20180706/";
const string MODEL_PATH = FILE_PATH+"model/";

//图像预处理,灰度，滤波
Mat pertImage0(IplImage* srcImage){
	Mat srcMat = cvarrToMat(srcImage);
	Mat grayImage;
	cvtColor(srcMat, grayImage, CV_BGR2GRAY);
	Mat blurImage;
	GaussianBlur(grayImage, blurImage, Size(3, 3), 0, 0);
	return blurImage;
}

//读文件时对每一行的处理
vector<string> exchange(string str, char* c){
	vector<string> resultVec;
	char *cstr, *p;
	cstr = new char[str.size() + 1];
	strcpy(cstr, str.c_str());
	p = strtok(cstr, c);
//	int i = 0;
	while (p)
	{
		resultVec.push_back(string(p));
//		ip_arr[i] = p;
		p = strtok(NULL, c);
//		i++;
	}
	delete[] cstr;
	return resultVec;
}

//截取图像
IplImage* getRectImage(IplImage *srcImage, int x, int y, int width, int height){
	cvSetImageROI(srcImage, cvRect(x, y, width, height));
	IplImage *rectImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	cvCopy(srcImage, rectImage, NULL);
	return rectImage;
}

//模板匹配
double temp_match(cv::Mat image, cv::Mat tepl, cv::Point &point, int method)
{
	int result_cols = image.cols - tepl.cols + 1;
	int result_rows = image.rows - tepl.rows + 1;

	cv::Mat result = cv::Mat(result_cols, result_rows, CV_32FC1);
	cv::matchTemplate(image, tepl, result, method);

	double minVal, maxVal;
	cv::Point minLoc, maxLoc;
	cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	switch (method)
	{
		case CV_TM_SQDIFF:
		case CV_TM_SQDIFF_NORMED:
			point = minLoc;
			return minVal;

		default:
			point = maxLoc;
			return maxVal;
	}
}

//计算两张图片的相似度
double GetSim(const Mat& src1, const Mat& src2){
	Mat hist1, hist2;
	int histSize = 256;
	float range[] = { 0, 256 };
	const float* histRange = { range };

	calcHist(&src1, 1, 0, Mat(), hist1, 1, &histSize, &histRange, 1, 0);
	normalize(hist1, hist1, 0, 1, NORM_MINMAX, -1, Mat());

	calcHist(&src2, 1, 0, Mat(), hist2, 1, &histSize, &histRange, 1, 0);
	normalize(hist2, hist2, 0, 1, NORM_MINMAX, -1, Mat());

	double Similarity = compareHist(hist1, hist2, CV_COMP_CORREL);
	return Similarity;
}

int match(){
	int model_num;
	cout<<"输入需要匹配模板编号：";
	cin>>model_num;
	//先读入模板model.txt
	ifstream fileinput;
    try {
        cout<<MODEL_PATH+to_string(model_num)+"/model.txt";
        fileinput.open(MODEL_PATH+to_string(model_num)+"/model.txt");
    }catch ( exception &e){
        cerr << "Caught: " << e.what( ) << endl;
        cerr << "Type: " << typeid( e ).name( ) << endl << endl;
    }

    if (!fileinput.is_open())
    {
        cout<<"打开模板文件失败！"<<endl;
        return 0;
    }
	string data[1];
	fileinput >> data[0];
	fileinput.close();

	//大圆的坐标
//	CvRect squareRect;
//	string squareRectValue = exchange(data[0], ":", 1);
//	squareRect.x = atoi(exchange(squareRectValue, ",", 0));
//	squareRect.y = atoi(exchange(squareRectValue, ",", 1));
//	squareRect.width = atoi(exchange(squareRectValue, ",", 2));//width和height是一样的
//	squareRect.height = atoi(exchange(squareRectValue, ",", 3));

	//待匹配区域的坐标
	CvRect matchRect;
	string matchRectValue = exchange(data[0], ":")[1];
	vector<string> matchs = exchange(matchRectValue, ",");


	matchRect.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
	matchRect.y = atoi(matchs[1].c_str());
	matchRect.width = atoi(matchs[2].c_str());
	matchRect.height = atoi(matchs[3].c_str());

	//读入图片到容器中
	Mat modelMat;
	unsigned int index = 0;
	unsigned int total = 360;
	char filename[100];
	vector<Mat> modelMats;
	IplImage* matchImage = cvCreateImage(cvSize(matchRect.width, matchRect.height), IPL_DEPTH_8U, 1);
	for (index; index < total; index++){
		sprintf(filename, (MODEL_PATH+to_string(model_num)+"/"+to_string(index)+".jpg").c_str());
		matchImage = cvLoadImage(filename, 1);
		if (matchImage != NULL){
			Mat modelMat = cvarrToMat(matchImage);
			modelMats.push_back(modelMat);
			//index = index + 5;
		}
		else{
			cout<<model_num<< "号模板图像不足360张！" << endl;
			break;
		}
	}
	//待测图像,这里换成摄像头代码
	IplImage* matchSrcImage = cvLoadImage((FILE_PATH+"15.jpg").c_str(), 1);
	//IplImage* grayImage = cvCreateImage(cvSize(matchSrcImage->width, matchSrcImage->height), IPL_DEPTH_8U, 1);
	//cvCvtColor(matchSrcImage, grayImage, CV_RGB2GRAY);
	//高斯滤波预处理
	Mat blurMatchDstMat = pertImage0(matchSrcImage);
	IplImage* matchBlurImage = cvCreateImage(cvGetSize(matchSrcImage), IPL_DEPTH_8U, 1);
	IplImage temp_1 = (IplImage)blurMatchDstMat;
	matchBlurImage = &temp_1;

	//截图
//	IplImage *squareRectImage = getRectImage(matchBlurImage, squareRect.x, squareRect.y, squareRect.width, squareRect.height);
	//截匹配区域
	IplImage* matchRectImage = getRectImage(matchBlurImage, matchRect.x, matchRect.y, matchRect.width, matchRect.height);
	//二值化,不需要二值化
	//Mat srcMatchRect = cvarrToMat(squareRectImage);
	Mat dstMatchRect = cvarrToMat(matchRectImage);
	//threshold(srcMatchRect, dstMatchRect, 0, 255, CV_THRESH_OTSU);
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
		iter++;
		//char filename[100];
		//sprintf(filename, "2018422/1/model%d.jpg", vect_index);
		//imshow(filename, *it);
		//++vect_index;
	}
	Time = (double)cvGetTickCount() - Time;
	printf("match run time = %gms\n", 36 * Time / (cvGetTickFrequency() * 1000));//毫秒
	printf("正确旋转角度 = %d\n ", angle);

	cvWaitKey(0);
	getchar();
	return 1;
}