#include "match.h"

const string FILE_PATH = "../20180706/";
const string MODEL_PATH = FILE_PATH+"model/";


Mat illumination0(Mat bgr_image){
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
	// display the results  (you might also want to see lab_planes[0] before and after).
//    cv::imshow("image original", bgr_image);
//    cv::imshow("image CLAHE", image_clahe);
//    cv::waitKey();
}


//白平衡
Mat whiteBalance0(Mat g_srcImage){
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

void unevenLightCompensate(Mat &image, int blockSize)
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
	dst.convertTo(image, CV_8UC3);
}


//图像预处理,灰度，滤波
Mat pertImage0(Mat srcImage){
	Mat grayImage;
	Mat blurImage;
	srcImage=whiteBalance0(srcImage);
	unevenLightCompensate(srcImage,30);  //返回灰度图
//    srcImage = illumination0(srcImage);

	if(srcImage.channels()==3)
		cvtColor(srcImage, grayImage, CV_BGR2GRAY);
	else
		grayImage=srcImage.clone();
//    cv::normalize(grayImage, grayImage, 0,1, cv::NORM_L2);
//	convertScaleAbs(grayImage,grayImage);
//	Mat gainMat(grayImage.rows, grayImage.cols, CV_8UC1, Scalar::all(255));

	equalizeHist(grayImage,grayImage);
//	bilateralFilter(grayImage, blurImage, 5, 20, 3);
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

