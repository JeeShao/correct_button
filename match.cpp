#include "match.h"

const string FILE_PATH = "../20180706/";
const string MODEL_PATH = FILE_PATH+"model/";

//图像预处理,灰度，滤波
Mat pertImage0(Mat srcImage){
	Mat grayImage;
	cvtColor(srcImage, grayImage, CV_BGR2GRAY);
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

