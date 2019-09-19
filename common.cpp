#include "common.h"

double RADIUS=15.30;
int SYMMERY = 0; //0-不对称 1-对称
int HOLES = 4;
int POSITION = 3;
int EXPOSURE = 1200;
int GAIN = 255;
string ANGLE_SHOW = "";
string STATUS_SHOW = "正常";

const string FILE_PATH = "20180706/";
const string MODEL_PATH = FILE_PATH+"model/";
const string ORG_PATH = FILE_PATH+"org_imgs/";
const string RECT_PATH = FILE_PATH+"org_rects/"; //模板图矩形框
const string PARAMS_PATH = FILE_PATH+"params.txt"; //参数文件
const string INIT_FILE = FILE_PATH+"init.txt";
const string LOG_FILE = FILE_PATH+"log.txt";
const string LOGO_FILE = FILE_PATH+"logo.jpg";
/*************************************
Function:    remove_files()
Description: 删除目录下所有文件
Parameters:  path:目录路径
Return:      bool-
**************************************/
bool remove_files(const char* path){
    DIR* dir = opendir(path);//打开指定目录
    dirent* p = NULL;//定义遍历指针
    try{
        while((p = readdir(dir)) != NULL)//开始逐个遍历
        {
            //过滤掉目录中"."和".."隐藏文件
            if(p->d_name[0] != '.')//d_name是一个char数组，存放当前遍历到的文件名
            {
                string name = string(p->d_name);
                remove((path+name).c_str());
//                cout<<(path+name).c_str()<<"000"<<endl;
            }
        }
        closedir(dir);
    }catch (exception &e){
        cerr << "Caught: " << e.what( ) << endl;
        cerr << "Type: " << typeid( e ).name( ) << endl << endl;
        return false;
    }
    return true;
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

//图像预处理,灰度，滤波
Mat pertImage(Mat srcImage){
    Mat grayImage;
    Mat blurImage;
    if(srcImage.channels()==3) {
        srcImage = whiteBalance(srcImage);
        cvtColor(srcImage, grayImage, CV_BGR2GRAY);
    } else
        grayImage = srcImage.clone();

//    cv::normalize(grayImage, grayImage, 0,1, cv::NORM_L2);
//	convertScaleAbs(grayImage,grayImage);
//	Mat gainMat(grayImage.rows, grayImage.cols, CV_8UC1, Scalar::all(255));

//	equalizeHist(grayImage,grayImage);

//    const int maxVal = 255;
//    int blockSize = 3;	//取值3、5、7....等
//    int constValue = 5;
//    int adaptiveMethod = 0;
//    int thresholdType = 1;
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

Mat gauss_diff(Mat gray){
    Mat gauss,diff,tmp;
    GaussianBlur(gray, gauss, cv::Size(3,3),0,0);
    diff = gray-gauss;
    tmp = gray+10*diff;
    return tmp;
}

//获取旋转后图像
vector<Mat> rotation(Mat img,Point center)
{
    vector<Mat> rotation_imgs;
    Mat map_matrix,res,dst;

//    for(int i=-2;i<=2;i+=2){ //3度
//        map_matrix=getRotationMatrix2D(center, i, 1.0);  //旋转中心，角度，缩放比例
//        warpAffine(img,dst,map_matrix,Size(img.cols,img.rows));
//        res = dst.clone();
//        rotation_imgs.push_back(res);
//    }

    for(int i=-6;i<=6;i+=2){ //7度
        map_matrix=getRotationMatrix2D(center, i, 1.0);  //旋转中心，角度，缩放比例
        warpAffine(img,dst,map_matrix,Size(img.cols,img.rows));
        res = dst.clone();
        rotation_imgs.push_back(res);
    }


    return rotation_imgs;
}

//获取平移后图像
vector<Mat> move1(vector<Mat> rotate_imgs,Rect rect,int flag)
{
    Mat res;
    vector<Mat> move_imgs;
    Rect match=rect;
    for(vector<Mat>::iterator it=rotate_imgs.begin();it!=rotate_imgs.end();it++){//7张旋转图
        for(int i=-6;i<=6;i+=3){//5张平移
//        for(int i=-3;i<=3;i+=3){//3张平移
            if(flag==0)//上下矩形
            {match.y=rect.y+i;}
            else
            {match.x=rect.x+i;}
            res = (*it)(match).clone();
            if(GAUSS_DIFF)
                res = gauss_diff(res); //高反差
            move_imgs.push_back(res);
        }
    }
    return move_imgs;
}

//拉普拉斯增强
Mat laplace_enhance(Mat gray) {
    if (gray.empty())
    {
        cerr << "打开图片失败,请检查" << std::endl;
    }
    Mat imageEnhance;
    typedef cv::Matx<double, 3, 3> kernel;
    kernel m(0, -1, 0, 0, 3, 0, 0, -1, 0);
//    Mat kernel = (Mat_<uchar>(3, 3) << 0, -1, 0, 0, 5, 0, 0, -1, 0);

    filter2D(gray, imageEnhance, CV_8UC1, m);
//    imshow("原图像", gray);
//    imshow("拉普拉斯算子图像增强效果", imageEnhance);
//    waitKey(0);
    return imageEnhance;
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

string get_time( ) {
    string datetime;
    time_t now = time(0);
    tm *ltm = localtime(&now);

    string mon = (1+ltm->tm_mon) <10? '0'+to_string(1+ltm->tm_mon):to_string(1+ltm->tm_mon);
    string day = ltm->tm_mday <10? '0'+to_string(ltm->tm_mday):to_string(ltm->tm_mday);
    string hour = ltm->tm_hour <10? '0'+to_string(ltm->tm_hour):to_string(ltm->tm_hour);
    string min = ltm->tm_min <10? '0'+to_string(ltm->tm_min):to_string(ltm->tm_min);

    datetime = to_string(1900+ltm->tm_year) + mon+ day + hour + min;
    return datetime;
}

bool init_sys(){
    string start = "201908202023";
    string end   = "201909302023";
    string now   = get_time();
    if(now>end || now<start)
        return 1;
    else
        return 0;
}

void getDate(char *dateNow) {
    string datetime;
    time_t now = time(0);
    tm *ltm = localtime(&now);

    string mon = (1+ltm->tm_mon) <10? '0'+to_string(1+ltm->tm_mon):to_string(1+ltm->tm_mon);
    string day = ltm->tm_mday <10? '0'+to_string(ltm->tm_mday):to_string(ltm->tm_mday);

    datetime = to_string(1900+ltm->tm_year) + mon+ day;
    sprintf(dateNow,"%s", (char*)datetime.c_str());
}

static int getdiskid(char *hardc) {
    int fd;
    char serialNo[8];//8位
    struct hd_driveid hid;
    fd = open ("/dev/sda", O_RDONLY);
    if (fd < 0)
    {
        return -1;
    }
    if (ioctl (fd, HDIO_GET_IDENTITY, &hid) < 0)
    {
        return -1;
    }

    close(fd);
    for(int i=12;i<20;i++){
        serialNo[i-12] = hid.serial_no[i]<71? hid.serial_no[i]:(hid.serial_no[i]%10+48);
    }
//    for(int i=0;i<8;i++){
//      serialNo[i] = hid.serial_no[i]<71? hid.serial_no[i]:(hid.serial_no[i]%10+48);
//    }
    // serialNo = string((char*)hid.serial_no).substr(12,8).c_str();
    sprintf(hardc,"%s", serialNo);
    //printf("当前序列号：%s\n",serialNo);
    return 0;
}

int char2int(char input) {
    return input>64?(input-55):(input-48);
}

int int2char(char input) {
    return input>9?(input+55):(input+48);
}

void hexStrXor(char * HexStr1, char * HexStr2, char * HexStr ) {
    int i, iHexStr1Len, iHexStr2Len, iHexStrLenLow, iHexStrLenGap;
    //转换成大写并求长度
    iHexStr1Len = strlen( HexStr1 );
    iHexStr2Len = strlen( HexStr2 );
    //获取最小的长度
    iHexStrLenLow = iHexStr1Len<iHexStr2Len?iHexStr1Len:iHexStr2Len;
    //获取长度差值
    iHexStrLenGap = abs( iHexStr1Len-iHexStr2Len );
    //两个十六进制的字符串进行异或
    for( i=0; i<iHexStrLenLow; i++ )
    {
        *(HexStr+i) = char2int( HexStr1[i] ) ^ char2int( HexStr2[i] );
        *(HexStr+i) = int2char( *(HexStr+i) );
    }
    if( iHexStr1Len>iHexStr2Len )
        memcpy( HexStr+i, HexStr1+i, iHexStrLenGap );
    else if( iHexStr1Len<iHexStr2Len )
        memcpy( HexStr+i, HexStr2+i, iHexStrLenGap );
    *( HexStr+iHexStrLenLow+iHexStrLenGap ) = 0x00;
}

bool systemCheck(){
    char nowDate[8];
    char hardseri[8];
    char xorLastRes[8];
    char xorEndRes[8];
    char lastDate[8];
    char endDate[8];
    fstream fileinput;
    try {
        fileinput.open("/bin/.sysdisk.dat");
    }catch ( exception &e){
        cerr << "Caught: " << e.what( ) << endl;
        cerr << "Type: " << typeid( e ).name( ) << endl << endl;
    }
    if (!fileinput.is_open())
    {
        cout<<"打开文件失败！"<<endl;
        return false;
    }

    string data[2];
    for(int i=0;i<2;i++){
        fileinput>>data[i];
    }
    fileinput.close();

    sprintf(xorLastRes,"%s",exchange(data[0], "=")[1].c_str());
    sprintf(xorEndRes,"%s",exchange(data[1], "=")[1].c_str());
    getDate(nowDate);
    if(getdiskid(hardseri)==-1)//失败
    {
        cout<<"序列号获取失败"<<endl;
        return false;
    }
    hexStrXor(xorLastRes,hardseri,lastDate);
    hexStrXor(xorEndRes,hardseri,endDate);
//    printf("最近一次:%s\n",lastDate);
//    printf("截止时间:%s\n",endDate);
//    printf("当前时间:%s\n",nowDate);
//    cout<<"strcmp(dateNow,lastTime)="<<strcmp(nowDate,lastDate)<<endl;
//    cout<<"strcmp(dateNow,endTime)="<<strcmp(nowDate,endDate)<<endl;
    if(strcmp(nowDate,lastDate)<0 || strcmp(nowDate,endDate)>0){
        remove("/bin/.sysdisk.dat");
        return false;
    }
    try {
        fileinput.open("/bin/.sysdisk.dat");
    }catch ( exception &e){
        cerr << "Caught: " << e.what( ) << endl;
        cerr << "Type: " << typeid( e ).name( ) << endl << endl;
    }
    if (!fileinput.is_open())
    {
        //cerr<<"打开文件失败！"<<endl;
        return false;
    }
    hexStrXor(nowDate,hardseri,xorLastRes);//更新
    fileinput << "lastserial="<<xorLastRes<<endl;
    fileinput << "endserial="<<xorEndRes<<endl;
    //cout<<"写入成功"<<endl;
    return true;
}

void writeLog(const char* logStr) {
    string datetime;
    time_t now = time(0);
    tm *ltm = localtime(&now);

    string mon = (1+ltm->tm_mon) <10? '0'+to_string(1+ltm->tm_mon):to_string(1+ltm->tm_mon);
    string day = ltm->tm_mday <10? '0'+to_string(ltm->tm_mday):to_string(ltm->tm_mday);
    string hour = ltm->tm_hour <10? '0'+to_string(ltm->tm_hour):to_string(ltm->tm_hour);
    string min = ltm->tm_min <10? '0'+to_string(ltm->tm_min):to_string(ltm->tm_min);
    string second = ltm->tm_sec<10? '0'+to_string(ltm->tm_sec):to_string(ltm->tm_sec);
    datetime = to_string(1900+ltm->tm_year) +'/'+ mon+'/'+ day+' ' + hour +':' +min+':'+second;
    ofstream logFile;
    logFile.open(LOG_FILE,ios::out|ios::in|ios::app);
    logFile<<'['<<datetime<<"]:"<<logStr<<endl;
    logFile.flush();
    logFile.seekp(0,logFile.end);
    if(logFile.tellp()>100*1024){//内存达到100M清空
        remove(LOG_FILE.c_str());
        cout<<"删除log文件"<<endl;
    }
    logFile.close();
}

void save_params(){

    fstream file;
    try{
        file.open(PARAMS_PATH.c_str(),ios::out);
        file.clear();//先清空文件
    }catch (exception &e){
        cerr<<e.what()<<endl;
        cerr<<"参数文件params.txt打开失败!"<<endl;
        exit(0);
    }

    file << "RADIUS:"<<RADIUS<<endl;
    file << "SYMMERY:"<<SYMMERY<<endl;
    file << "HOLES:"<<HOLES<<endl;
    file << "POSITION:"<<POSITION<<endl;
    file << "EXPOSURE:"<<EXPOSURE<<endl;
    file << "GAIN:"<<GAIN<<endl;
    file.close();
}

void read_params(){
    ifstream fileinput;
    try {
        fileinput.open(PARAMS_PATH.c_str());
    }catch ( exception &e){
        cerr << "Caught: " << e.what( ) << endl;
        cerr << "Type: " << typeid( e ).name( ) << endl << endl;
    }
    if (!fileinput.is_open())
    {
        cerr<<"打开params.txt文件失败！"<<endl;
        exit(0);
    }

    string data[6];
    for(int i=0;i<6;i++){
        fileinput>>data[i];
    }
    RADIUS = atof((exchange(data[0], ":")[1]).c_str());
    SYMMERY = atoi((exchange(data[1], ":")[1]).c_str());
    HOLES = atoi((exchange(data[2], ":")[1]).c_str());
    POSITION = atoi((exchange(data[3], ":")[1]).c_str());
    EXPOSURE = atoi((exchange(data[4], ":")[1]).c_str());
    GAIN = atoi((exchange(data[5], ":")[1]).c_str());
    fileinput.close();
}

