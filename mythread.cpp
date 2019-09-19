#include "mythread.h"

QMutex mutex;
QWaitCondition finishCond;
volatile int FINISH_TAKE_PHOTO = 0; //完成拍照key
Mat PHOTO;

unsigned char MyThread::DATA_MSG[5] = {0xDE,0xA9,00,0xFF,0xFF};  //通知
unsigned char MyThread::DATA_ANGLE[6] = {0xDE,0xA1,01,00,0xFF,0xFF}; //旋转角度
unsigned char MyThread::DATA_REC[8] = {}; //接受串口信息

MyThread::MyThread(QObject *parent):QThread(parent)
{

}

MyThread::~MyThread()
{
    qDebug()<<"线程结束...";
    quit();
    wait();
}

void MyThread::run()
{
    int nbyte=0;
    int match_pattern=-1;//识别程序返回值  1：模板生成  2：识别模式
    int generate_pattern=-1;//模板生成程序返回值  2：识别模式
    int t=1;
    //读取参数
    read_params();
    //保存参数
    save_params();

    //初始化系统
    if(init_sys())
        return;
    while(initPtty()!=1)//串口异常
    {
        if(STATUS_SHOW != "串口异常")
        {
            STATUS_SHOW = "串口异常";
            emit updateStatus(STATUS_SHOW);
        }
        msleep(10);//10ms
    }
    match_pattern = match();//默认进入识别模式
    if (match_pattern==1)
    {generate_pattern=generate_temp();match_pattern=-1;}
    else if(match_pattern==0)
    {init_param(RADIUS);match_pattern=-1;}

    Mat img; //相机读取的图像
    while(1){
            if (match_pattern==1)
            {generate_temp();match_pattern=-1;}
            else if(match_pattern==0)
            {init_param(RADIUS);match_pattern=-1;}

            if (generate_pattern==2)
            {match_pattern=match();generate_pattern=-1;}

            memset(DATA_REC,0,8);
            nbyte=recvnTTY(ptty,DATA_REC,5);
            if(nbyte==5){//设置参数
                setParams();
            }
            if(t==1)
            {if(DEBUG) cout<<"等待串口数据……"<<endl;t++;}
            msleep(10);//10ms

            if(nbyte==4) {
                if(DEBUG)
                {
                    printData(DATA_REC,4);
                    cout<<endl;
                }

                switch (DATA_REC[1]) {
                    case 0xFF:{generate_temp();t=1;break;} //模板生成
                    case 0xFA:{match_pattern=match();t=1;break;} //方向识别
                    case 0xFB:{init_param(RADIUS);t=1;break;} //相机校正
                    case 0xF1:{if(DEBUG) cout<<"关机";system("shutdown -h now");}
                    default:{if(DEBUG) cerr<<"指令错误！默认进入识别模式"<<endl;writeLog("指令错误！默认进入识别模式");match_pattern=match();t=1;break;}
                }
            }
    }
}

//生成模板
int MyThread::generate_temp()
{
    ANGLE_SHOW="--";
    STATUS_SHOW="正常";
    emit updateStatus(STATUS_SHOW);
    emit updateAngle(ANGLE_SHOW);
//    remove_files(MODEL_PATH.c_str());//清除模板文件
    fcntl(ptty->fd, F_SETFL, FNDELAY);//非阻塞
    if(DEBUG)
        cout<<"进入模板生成程序……"<<endl;
    int cur_no=0;
    double btn_r=RADIUS;int holes=HOLES,position=POSITION;
    //从摄像头读取4个角度模板原图.保存至20180706/
    Mat frame;
    DATA_REC[1] = 0xFF;  //完成
    sendnTTY(ptty,DATA_REC,4);//准备好
    memset(DATA_REC,0,8);

    /*
     * 计算区域的参数
     */
    double button_radius = btn_r; //纽扣半径 mm
    Point center = Point(660,533); //纽扣中心
    double section_angle = PI/3; //每一个截取区域的角度 60度
    double pixs_mm = 18.43;//单位长度对应像素距离 = 18.53pixs/mm
    int left_x = 595;//4个铁钉外接矩形左边缘x
    int top_y  = 470;//4个铁钉外接矩形上边缘y
    int bottom_y = center.y+center.y-top_y;//4个铁钉外接矩形下边缘y
    int right_x = center.x+center.x-left_x;//4个铁钉外接矩形右边缘x
    int R = button_radius*pixs_mm; //纽扣像素半径
    ifstream fileinput;
    try {
        fileinput.open(INIT_FILE);
    }catch ( exception &e){
        cerr << "Caught: " << e.what( ) << endl;
        cerr << "Type: " << typeid( e ).name( ) << endl << endl;
        writeLog(e.what());
    }
    if (!fileinput.is_open())
    {
        cerr<<"打开初始化文件失败！"<<endl;
        writeLog("打开初始化文件失败");
        return -1;
        //exit(0);
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
            for (int i = 0; i < 2; i += 2) {
                rect_tests[i].x = rect_tests[i].x + 2 * margin;
                rect_tests[i].y = rect_tests[i].y + margin;
                rect_tests[i].width = rect_tests[i].width - 4 * margin;
                rect_tests[i].height = rect_tests[i].height - 2 * margin;

                rect_tests[i + 2].x = rect_tests[i + 2].x + 2 * margin;
                rect_tests[i + 2].y = rect_tests[i + 2].y + margin;
                rect_tests[i + 2].width = rect_tests[i + 2].width - 4 * margin;
                rect_tests[i + 2].height = rect_tests[i + 2].height - 2 * margin;
            }
        } else {
            for (int i = 0; i < 2; i += 2) {
                rect_tests[i].x = rect_tests[i].x + margin;
                rect_tests[i].y = rect_tests[i].y + 2 * margin;
                rect_tests[i].width = rect_tests[i].width - 2 * margin;
                rect_tests[i].height = rect_tests[i].height - 4 * margin;

                rect_tests[i + 2].x = rect_tests[i + 2].x + margin;
                rect_tests[i + 2].y = rect_tests[i + 2].y + 2 * margin;
                rect_tests[i + 2].width = rect_tests[i + 2].width - 2 * margin;
                rect_tests[i + 2].height = rect_tests[i + 2].height - 4 * margin;
            }
        }
        rect_tests[1] = rect_tests[2];
        rect_tests[2]=0x0;
        rect_tests[3]=0x0;
    }else{cerr<<"扣眼参数输入错误！"<<endl;writeLog("扣眼参数输入错误");return -1;}

    /*将匹配区域写入model.txt*/
    fstream file;
    try{
        file.open(MODEL_PATH+"/model.txt", ios::out);
        file.clear();//先清空文件
    }catch (exception &e){
        cerr<<e.what()<<endl;
        cerr<<MODEL_PATH+"/model.txt:模板文件打开失败!"<<endl;
        writeLog((MODEL_PATH+"/model.txt:模板文件打开失败!").c_str());
        return -1;
    }
    for(int i=0;i<holes;i++){
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
    Mat srcImage,img;
    int bytes=0;
    int capStatus=0;
    while(1){
        msleep(10);//10ms
        bytes = recvnTTY(ptty,DATA_REC,5);
        if(bytes==4)//拍照
        {
            if(DEBUG)
                printData(DATA_REC,4);
            if(DATA_REC[1]==0xFD)//拍照
            {
                capStatus =takePhoto();
                if(!capStatus){
                    if(STATUS_SHOW!="相机掉线")
                        sendCapLoseMsg();
                }
                else{
                    frame = PHOTO;
                    DATA_MSG[02]=03;
                    if(sendnTTY(ptty,DATA_MSG,5)==5){ //完成
                        if(DEBUG)
                            cout<<"模板图："<<cur_no<<endl;
                        imwrite((ORG_PATH+to_string(cur_no++)+".jpg").c_str(),frame);//图片保存到本工程目录中
                    }else
                        writeLog("send code error:串口发送失败");
                }
            }
            else if (DATA_REC[1] == 0xFB) {init_param(btn_r);}//进入相机校正模式
            else if(DATA_REC[1]==0xFE && cur_no<holes)  //退出模板
            {
                if(DEBUG)
                    cerr<<"模板生成失败,退出模板"<<endl;
                writeLog("模板生成失败,退出模板");
                return -1;
            }
            else if(DATA_REC[1]==0xFA) //进入识别模式
            {
                if(DEBUG)
                    cerr<<"模板生成失败"<<endl;
                writeLog("模板生成失败,进入识别模式");
                return 2;
            }
            else if(DATA_REC[1]==0xF1)
            {
                if(DEBUG) cout<<"关机";system("shutdown -h now");
            }
            memset(DATA_REC,0,8);
        }
        else if(bytes==5){//设置参数
            setParams();
            if(DEBUG)
                printData(DATA_REC,5);
            memset(DATA_REC,0,8);
        }
        else if(bytes!=-1){
            if(DEBUG)
                cout<<"取模板指令错误"<<endl;
            memset(DATA_REC,0,8);
        }
    if(cur_no>=holes)
        break;
    }

    //读取文件夹中的图像
    for (index; index < total; index++){
        sprintf(filename, (ORG_PATH+'/'+to_string(index)+".jpg").c_str());
        srcImage = imread(filename, 1);
        if (!srcImage.empty()){
            Mat blurModelDstMat = pertImage(srcImage);
            Mat matchImage ;
            //存入容器中
            matchImage = blurModelDstMat(rect_temps[index]).clone();
            modelImages.push_back(matchImage);
            rectangle(blurModelDstMat,rect_temps[index],Scalar(0,0,255),2,1,0);
            imwrite((RECT_PATH+to_string(index)+".jpg").c_str(), blurModelDstMat);
        }
        else{
            if(DEBUG)
                cout << "模板图像不足！" << endl;
            writeLog("模板图像不足");
            break;
        }
    }

    //取出匹配区域进行二值化并保存，其实可以不用存入容器中的，直接在上面二值化保存就行
    unsigned int vect_index = 0;
    for (vector<Mat>::iterator it = modelImages.begin(); it != modelImages.end(); it++)
    {
        Mat src = (*it).clone();
        sprintf(filename, (MODEL_PATH+"/"+to_string(vect_index)+".jpg").c_str());
        if(DEBUG)
            cout<<filename<<endl;
        if(GAUSS_DIFF)
            src=gauss_diff(src);//高反差
        if(LAPLACE)
            src=laplace_enhance(src);//拉普拉斯增强
        if(THRESH)
            adaptiveThreshold(src,src,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,25,5);
        if(BLUR)
        {GaussianBlur(src,src,Size(5,5),0,0);GaussianBlur(src,src,Size(17,17),0,0);}
        imwrite(filename, src);
        vect_index = vect_index + 1;
    }
    if(DEBUG)
        cout <<"模板生成成功！" << endl;
    return 0;
}

//角度识别
int MyThread::match()
{
    fcntl(ptty->fd, F_SETFL, FNDELAY);//非阻塞

    if(DEBUG)
        cout<<"进入方向识别程序……"<<endl;
    int holes=HOLES,position=POSITION;
    Mat frame;
    int nbyte=0;
    DATA_REC[1] = 0xFA;  //完成
    sendnTTY(ptty,DATA_REC,4);//准备好
    memset(DATA_REC,0,8);

    //先读model.txt
    ifstream fileinput;
    try {
        fileinput.open(MODEL_PATH+"/model.txt");
    }catch ( exception &e){
        cerr << "Caught: " << e.what( ) << endl;
        cerr << "Type: " << typeid( e ).name( ) << endl << endl;
        writeLog(e.what());
    }
    if (!fileinput.is_open())
    {
        if(DEBUG)
            cerr<<"模板不存在或打开模板文件失败！"<<endl;
        writeLog("模板不存在或打开模板文件失败！");
        return 0;
    }

    string data[holes+1];
    for(int i=0;i<holes+1;i++){
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
    }
    matchRectValue = exchange(data[holes], ":")[1];
    matchs = exchange(matchRectValue, ",");
    center.x = atoi(matchs[0].c_str());//取的字符矩形框比模板框稍大，这里没有取大，因为二值化的话，取大容易受影响
    center.y = atoi(matchs[1].c_str());

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
        else{ if(DEBUG) cerr<<"模板图像不足！" << endl;writeLog("模板图像不足！");return 0; }
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

    Mat img;
    ANGLE_SHOW="";
    int ii=0;
    while(1){
        direction=1;
        memset(DATA_REC,0,8);  //清空
        start = clock();
        nbyte = recvnTTY(ptty,DATA_REC,5);

        if(nbyte==4) {
            if(DEBUG){
                printData(DATA_REC, 4);
            }
            if (DATA_REC[1] == 0xFD)//拍照
            {
                if (!takePhoto()) {
                    if(STATUS_SHOW!="相机掉线")
                        sendCapLoseMsg();
                } else {
                    if(STATUS_SHOW != "正常"){
                        STATUS_SHOW = "正常";
                        emit updateStatus(STATUS_SHOW);//更新状态
                    }
                    frame = PHOTO;
                    max = 0;angle = 0;iter = 0;
                    blurMatchDstMat = pertImage(frame);
                    rotate_imgs = rotation(blurMatchDstMat, center); //旋转后7张图像
                    for (vector<Mat>::iterator it = modelMats.begin(); it != modelMats.end(); it++) {
                        rect_index = distance(modelMats.begin(), it);
                        move_imgs = move1(rotate_imgs, rects[rect_index], (rect_index + position) % 2);//rect旋转平移后35张
                        for (vector<Mat>::iterator r1 = move_imgs.begin(); r1 != move_imgs.end(); r1++) {//35张top
                            if (LAPLACE)
                                *r1 = laplace_enhance(*r1);//拉普拉斯增强
                            if (THRESH)
                                adaptiveThreshold(*r1, *r1, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 25, 5);
                            if (BLUR) {
                                GaussianBlur(*r1, *r1, Size(5, 5), 0, 0);
                                GaussianBlur(*r1, *r1, Size(17, 17), 0, 0);
                            }
                            value_tmp = temp_match(*it, *r1, matchLoc, TM_CCOEFF_NORMED);//TM_CCORR_NORMED
                            values.push_back(value_tmp);
                        }
                        value = *max_element(values.begin(), values.end());
//                            printf("%d angle,最大值 = %f\n ", iter, value);
                        if (max < value) {
                            max = value;
                            angle = iter;
                        }
                        iter += (360 / holes);
                        move_imgs.clear();
                        values.clear();
                    }

                    if (max >= 0.4 && max < 1.0)
                    {
                        if(STATUS_SHOW != "正常")
                        {
                            STATUS_SHOW = "正常";
                            emit updateStatus(STATUS_SHOW);
                        }
                        ANGLE_SHOW = to_string(SYMMERY ? (angle + 180) % 180 : angle);
                        DATA_ANGLE[2] = direction;
                        DATA_ANGLE[3] = SYMMERY ? (angle + 180) % 180 : angle;
                        sendnTTY(ptty, DATA_ANGLE, 6);
                        if(DEBUG)
                            printData(DATA_ANGLE, 6);
                    } else {//01图像无法使别
                        if(STATUS_SHOW != "无法识别")
                        {
                            STATUS_SHOW = "无法识别";
                            emit updateStatus(STATUS_SHOW);
                        }
                        ANGLE_SHOW = "ERROR";
                        DATA_MSG[2] = 01;
                        sendnTTY(ptty, DATA_MSG, 5);
                        if(DEBUG)
                        {
                            cout<<++ii<<endl;
                            printData(DATA_MSG, 5);
                        }
                    }
                    finish = clock();
                    total_time = (double) ((finish - start) * 1000 / CLOCKS_PER_SEC);//ms
                    printf("识别时间 = %gms\n", total_time);//毫秒
                    if(DEBUG){
                        total_time = (double) ((finish - start) * 1000 / CLOCKS_PER_SEC);//ms
                        printf("识别时间 = %gms\n", total_time);//毫秒
                    }
                    emit updateAngle(ANGLE_SHOW);
//                        imwrite(("./识别照片/" + to_string(ii++) + ".jpg").c_str(), frame);
                    if(DEBUG){
                        max >= 0.4 && max < 1.0 ? (direction == 0 ? printf("旋转角度 = %d ,方向 左\n", DATA_ANGLE[3]) : printf(
                                "旋转角度 = %d ,方向 右\n", DATA_ANGLE[3])) : printf("无法识别\n");
                        cout << endl;
//                        writeLog(STATUS_SHOW.c_str());
                    }
                }
            }
            else if (DATA_REC[1] == 0xFE) {return -1;} //退出识别
            else if (DATA_REC[1] == 0xFF) {return 1;} //进入模板生成模式
            else if (DATA_REC[1] == 0xFB) {return 0;}//进入相机校正模式
            else if (DATA_REC[1] == 0xF1) {if(DEBUG) cout<<"关机";system("shutdown -h now");}//关机
        }
        else if(nbyte==5){//设置参数
            setParams();
        }
//        else if(nbyte>0){
//            if(DEBUG){
//                printData(DATA_REC,4);
//                cout<<endl;
//            }
//            if(STATUS_SHOW != "异常指令")
//            {
//                STATUS_SHOW = "异常指令";
//                emit updateStatus(STATUS_SHOW);//更新状态
//            }
//            if(DEBUG)
//                cerr<<"异常指令"<<endl;
//            writeLog("异常指令[match()]");
//        }
        else
            msleep(10); //限制刷新
    }
}

//拍照
bool MyThread::takePhoto()
{
    clock_t start,finish;
    start = clock();
    emit takePhoto_signal();
    mutex.lock();
    while(FINISH_TAKE_PHOTO== 0)
    {
//        qDebug()<<"等到拍照完成...";
        finishCond.wait(&mutex);
    }
    mutex.unlock();
    if(FINISH_TAKE_PHOTO==1)
    {
//        qDebug()<<"拍照完成";
        FINISH_TAKE_PHOTO= 0;
        finish = clock();
//        cout<<"take "<<(double) ((finish - start) * 1000 / CLOCKS_PER_SEC)<<"ms"<<endl;//ms
        return true;
    }
    else{
//        qDebug()<<"相机掉线...";
        FINISH_TAKE_PHOTO= 0;
        return false;//相机掉线
    }
}

//串口初始化
inline int MyThread::initPtty()
{
    ptty = readyTTY(0);
    if(ptty == NULL) {
        printf("readyTTY(0) error\n");
        writeLog("readyTTY(0) error");
        return -1;
    }
    lockTTY(ptty);
    if(setTTYSpeed(ptty,9600)>0){  //设置波特率
        printf("setTTYSpeed() error\n");
        writeLog("setTTYSpeed(0) error");
        return -1;
    }
    if(setTTYParity(ptty,8,'N',1)>0){ //设置通讯格式
        printf("setTTYParity() error\n");
        writeLog("setTTYParity(0) error");
        return -1;
    }
    fcntl(ptty->fd, F_SETFL, FNDELAY);//非阻塞
    return 1;
}

//校正相机
void MyThread::init_param(double r)
{
    int n;
    r=RADIUS;
    cout<<"开始相机校正……"<<endl;
    Mat src, src_gray;
    vector<Vec3f> circles;

    while(circles.size()<=0){
        n = recvnTTY(ptty,DATA_REC,5);
        if(n==4 && DATA_REC[1]==0xFE){printData(DATA_REC,4);cout<<"退出校正"<<endl;return ;}//退出校正
        else if(n==4 && DATA_REC[1]==0xF1){if(DEBUG) cout<<"关机";system("shutdown -h now");}
        else if(n==5) {//设置参数
            printData(DATA_REC, 5);
            switch (DATA_REC[1]) {
                case 0xF4: {
                    EXPOSURE = DATA_REC[2] * 10;emit updateExposure(EXPOSURE);
                    cout << "曝光：" << DATA_REC[2] * 10 << endl;break;
                } //曝光
                case 0xF5: {
                    GAIN = DATA_REC[2];emit updateGain(GAIN);
                    cout << "增益：" << (int) DATA_REC[2] << endl;break;
                }  //增益
                case 0xFC:{RADIUS=DATA_REC[2]/10.0;r=RADIUS;break;}  //直径
            }
            save_params();
        }
        if(!takePhoto() && STATUS_SHOW!="相机掉线"){
            sendCapLoseMsg();
        }
        src = PHOTO;
        if( !src.data )
        { cout<<"未读取到摄像头图像!"<<endl;writeLog("未读取到摄像头图像");return ; }

        cvtColor( src, src_gray, CV_BGR2GRAY );
        GaussianBlur( src_gray, src_gray, Size(9, 9), 2, 2 );
        HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, src_gray.rows/8, 100, 50, src_gray.rows/8, src_gray.rows/3 );
        cout<<"检测到"<<circles.size()<<"个圆"<<endl;
    }

    for( size_t i = 0; i < circles.size(); i++ )
    {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        //圆心
        circle( src, center, 3, Scalar(0,255,0), -1, 8, 0 );
        //圆
        circle( src, center, radius, Scalar(0,0,255), 3, 8, 0 );
    }
    imwrite("init.jpg",src);
    emit showInitImg(src.clone());
    if(circles.size()>1){
        cerr<<"校正失败"<<endl;
        writeLog("校正失败");
        return ;
    }
    Point center = Point((int)circles[0][0],(int)circles[0][1]); //纽扣中心
    double pixs_mm = 1.0*circles[0][2]/r;//单位长度对应像素距离 = 18.53pixs/mm
    int left_x = center.x-X_MARGIN;//4个铁钉外接矩形左边缘x
    int top_y  = center.y-Y_MARGIN;//4个铁钉外接矩形上边缘y
    int bottom_y = center.y+Y_MARGIN;//4个铁钉外接矩形下边缘y
    int right_x = center.x+X_MARGIN;//4个铁钉外接矩形右边缘x

    fstream file;
    try{
        file.open(INIT_FILE, ios::out);
        file.clear();//先清空文件
    }catch (exception &e){
        cerr<<e.what()<<endl;
        cerr<<"初始化文件打开失败!"<<endl;
        writeLog("initParams():初始化文件打开失败");
        return;
    }
    file << "center:"<<center.x<<","<<center.y<<endl;
    file << "top_y:"<<top_y<< endl;
    file << "left_x:"<<left_x<< endl;
    file << "bottom_y:"<<bottom_y<< endl;
    file << "right_x:"<<right_x<< endl;
    file << "pixs_mm(像素比):"<<pixs_mm<< endl;
    file.close();
    cout<<"校正完成……"<<endl;
}

//发送相机掉线信息
void MyThread::sendCapLoseMsg()
{
    STATUS_SHOW="相机掉线";
    emit updateStatus(STATUS_SHOW);//更新状态
    DATA_MSG[2]=02;
    sendnTTY(ptty,DATA_MSG,5);
    if(DEBUG)
        {cout<<"掉线--";printData(DATA_MSG,5);}
}

//响应设置参数指令
void MyThread::setParams()
{
    if(DEBUG)
        printData(DATA_REC,5);
    switch (DATA_REC[1]){
        case 0xF4:{EXPOSURE=DATA_REC[2]*10;emit updateExposure(DATA_REC[2]*10);cout<<"曝光："<<DATA_REC[2]*10<<endl;break;} //曝光
        case 0xF5:{GAIN=DATA_REC[2];emit updateGain(DATA_REC[2]);cout<<"增益："<<(int)DATA_REC[2]<<endl;break;}  //增益
        case 0xF8:{HOLES=DATA_REC[2];cout<<"扣眼："<<(int)DATA_REC[2]<<endl;break;}  //扣眼
        case 0xF7:{POSITION=(DATA_REC[2]+3)%4;cout<<"字符位置："<<(int)DATA_REC[2]<<endl;break;}  //字符位置
        case 0xFC:{RADIUS=DATA_REC[2]/10.0;break;}  //直径
        case 0xF6:{SYMMERY=DATA_REC[2];break;}  //对称
        default:break;
    }
    save_params();
}

//打印串口数据
void MyThread::printData(unsigned char *arr,int size)
{
    for(int i=0;i<size;i++) {
        printf("%02X ",arr[i]);
        fflush(stdout);
    }
    cout<<endl;
}

//相机掉线
void MyThread::lossCamera()
{
    if(STATUS_SHOW!="相机掉线")
        sendCapLoseMsg();
}