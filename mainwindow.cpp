#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent),img_no(0)
{
    CapInit = false;
    //屏幕分辨率
    screen_w=768;
    screen_h=1024;
    //中心线
    start1 = Point(width/2,height/2-100);
    end1 = Point(width/2,height/2+100);
    start2 = Point(width/2-100,height/2);
    end2 = Point(width/2+100,height/2);
    //90度
    matrix.rotate(90.0);

    mythread = new MyThread(this);
    //××××××××××更改相机驱动××××××××
    //cap = new Capture(200);cap->open();
    cap = new Camera(1280,1024);
    //××××××××××××××××××××××××××××
    this->setupUi(this);
}

MainWindow::~MainWindow()
{
    cap->close();//释放相机
    delete centralwidget;
}

//初始化UI
void MainWindow::setupUi(QMainWindow *MainWindow)
{
    //状态，角度字体
    font1.setFamily(QString::fromUtf8("\346\245\267\344\275\223"));
    font1.setPointSize(25);
    font1.setBold(true);

    if (MainWindow->objectName().isEmpty())
        MainWindow->setObjectName("MainWindow");
    MainWindow->resize(QApplication::desktop()->height(), QApplication::desktop()->width());
//    MainWindow->setFixedSize(QApplication::desktop()->height(), QApplication::desktop()->width());
    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName("centralwidget");

    //logo
    label_logo = new QLabel(centralwidget);
    label_logo->setObjectName("label_logo");
    label_logo->setGeometry(QRect(120, 81, 131*0.5, 149*0.5*0.75));// 131*149
    label_logo->setAlignment(Qt::AlignCenter);
    label_logo->setScaledContents(true);
    QPixmap pix = QPixmap(LOGO_FILE.c_str());
    label_logo->setPixmap(pix);

    //公司名称
    label_title = new QLabel(centralwidget);
    label_title->setObjectName("label_title");
    label_title->setGeometry(QRect(206, 91, 450, 50*0.75));
    QFont font;
    font.setFamily(QString::fromUtf8("\346\245\267\344\275\223"));
    font.setPointSize(28);
    font.setBold(true);
    font.setWeight(75);
    label_title->setFont(font);

    //显示画面
    label_frame = new QLabel(centralwidget);
    label_frame->setObjectName("label_frame");
    label_frame->setGeometry(QRect(0, 152, screen_w, (double)width/height*screen_w*0.75));
    label_frame->setLayoutDirection(Qt::LeftToRight);
    label_frame->setScaledContents(true); //图片自适应窗口大小
    label_frame->setAlignment(Qt::AlignCenter);
    label_frame->setStyleSheet("color:red;");
    label_frame->setFont(font1);

    //状态显示
    label_status = new QLabel(centralwidget);
    label_status->setObjectName("label_status");
    label_status->setGeometry(QRect(80, 892, 80, 50*0.75));
    label_status->setFont(font1);
    //    label_angle->setWindowOpacity(1);//透明度 1-不透明
    label_status->setWindowFlags(Qt::FramelessWindowHint);
    label_status->setAttribute(Qt::WA_TranslucentBackground);
    //状态值
    label_status_val = new QLabel(centralwidget);
    label_status_val->setObjectName("label_status_val");
    label_status_val->setGeometry(QRect(180, 892, 150, 50*0.75));
    label_status_val->setFont(font1);
    label_status_val->setStyleSheet("color:green;");
    label_status_val->setWindowFlags(Qt::FramelessWindowHint);
    label_status_val->setAttribute(Qt::WA_TranslucentBackground);


    //角度显示
    label_angle = new QLabel(centralwidget);
    label_angle->setObjectName("label_angle");
    label_angle->setGeometry(QRect(464, 892, 160, 50*0.75));
    label_angle->setFont(font1);
    label_angle->setWindowFlags(Qt::FramelessWindowHint);
    label_angle->setAttribute(Qt::WA_TranslucentBackground);

    //角度值
    label_angle_val = new QLabel(centralwidget);
    label_angle_val->setObjectName("label_angle_val");
    label_angle_val->setGeometry(QRect(620, 892, 145, 50*0.75));
    label_angle_val->setFont(font1);
    label_angle_val->setWindowOpacity(0);//透明度 1-不透明
    label_angle_val->setWindowFlags(Qt::FramelessWindowHint);
    label_angle_val->setAttribute(Qt::WA_TranslucentBackground);

    MainWindow->setCentralWidget(centralwidget);
    retranslateUi(MainWindow);


    read_params();//读取曝光和增益参数
    if(!cap->init(EXPOSURE,GAIN)){ //启动时未插上相机
        label_frame->setText("连接相机失败");
        usleep(10);
    } else
        CapInit = true;
    sleep(2);//等待相机就位
    timer1 = new QTimer(this);
    connect(timer1,SIGNAL(timeout()),this,SLOT(show_frame()));
    connect(mythread,&MyThread::takePhoto_signal,this,&MainWindow::dealTakePhoto,Qt::DirectConnection);
    connect(mythread,SIGNAL(updateStatus(string)),this,SLOT(setStatus(string)));
    connect(mythread,SIGNAL(updateAngle(string)),this,SLOT(setAngle(string)));
    connect(mythread,SIGNAL(updateExposure(int)),this,SLOT(setExposure(int)));
    connect(mythread,SIGNAL(updateGain(int)),this,SLOT(setGain(int)));
    connect(this,SIGNAL(cameraLoss()),mythread,SLOT(lossCamera()));
    connect(mythread,SIGNAL(cameraLoss()),mythread,SLOT(lossCamera()));
    connect(mythread,SIGNAL(showInitImg(Mat)),this,SLOT(setShowInitImg(Mat)));
    timer1->start(30);//30ms
    mythread->start();
}

//设置控件内容
void MainWindow::retranslateUi(QMainWindow *MainWindow)
{
    MainWindow->setWindowTitle("实时图像");
    MainWindow->setStyleSheet("background-color: rgba( 192, 233, 255, 100% );");
//    MainWindow->setStyleSheet("background-image: url(../20180706/bg.jpg);");
    label_title->setText("上海鸠兹智能科技有限公司");
    label_status->setText("状态:");
    label_status_val->setText(QString(STATUS_SHOW.c_str()));
    label_angle->setText("识别角度:");
    label_angle_val->setText(QString("0"));
}

//slots 显示图像
void MainWindow::show_frame()
{
    if(CapInit){
        if(cap->read())
        {
            image=cap->frame;
            line(image,start1,end1,Scalar(45, 35, 255),2);
            line(image,start2,end2,Scalar(45, 35, 255),2);
            label_frame->setPixmap(QPixmap::fromImage(mat2QImage(image).transformed(matrix,Qt::FastTransformation)));
        }else{
            if(DEBUG)
                cout<<"掉线拉...."<<endl;
//            emit mythread->updateStatus("掉线00");
            emit mythread->cameraLoss();
//            label_frame->setText("相机掉线");
        }
    }
    else{
        if(!cap->init(EXPOSURE,GAIN)){ //启动时未插上相机
            label_frame->setText("连接相机失败");
//            emit mythread->updateStatus("掉线11");
            emit mythread->cameraLoss();
//            emit this->cameraLoss();
            usleep(10);
        } else
            CapInit = true;
    }
}

//slots 拍照
void MainWindow::dealTakePhoto()
{
//    qDebug()<<"拍照";
    if(1)
    {
        mutex.lock();
        FINISH_TAKE_PHOTO = 1;
        mutex.unlock();
        PHOTO = cap->frame;
        finishCond.wakeAll();
//        qDebug()<<"唤醒线程";
    }
    else{
        mutex.lock();
        FINISH_TAKE_PHOTO = -1;
        mutex.unlock();
        PHOTO = cap->frame;
        finishCond.wakeAll();
//        qDebug()<<"无法拍照";
    }

}

//slots 设置显示状态
void MainWindow::setStatus(string status)
{
    if(status=="正常")
    {
        label_status_val->setStyleSheet("color:green;");
    }
    else{
        label_status_val->setStyleSheet("color:red;");
    }
    label_status_val->clear();
    label_status_val->setText(QString(status.c_str()));
}

//slots 设置显示角度
void MainWindow::setAngle(string angle)
{
    label_angle_val->clear();
    label_angle_val->setText(QString(angle.c_str()));
}

//slots 设置曝光
void MainWindow::setExposure(int val)
{
    cap->setExposure(val);
}

//slots 设置增益
void MainWindow::setGain(int val)
{
    cap->setGain(val);
}

//slots 相机校正时显示校正结果
void MainWindow::setShowInitImg(Mat img)
{
//    line(img,start1,end1,Scalar(45, 35, 255),2);
//    line(img,start2,end2,Scalar(45, 35, 255),2);
    QPixmap pixmap= QPixmap::fromImage(mat2QImage(img).transformed(matrix,Qt::FastTransformation));
    label_frame->setPixmap(pixmap);
    label_frame->repaint();//立即重绘  update()是加入到时间列表
    QCoreApplication::processEvents();
    QThread::sleep(2);
}

//Mat转QImage
QImage MainWindow::mat2QImage(Mat cvImg)
{
    QImage qImg;
    if (cvImg.channels() == 3)  //3 channels color image
    {
        qImg = QImage((const unsigned char*)(cvImg.data),
                      cvImg.cols, cvImg.rows,
                      cvImg.cols*cvImg.channels(),
                      QImage::Format_RGB888);
        qImg = qImg.rgbSwapped();
    }
    else if (cvImg.channels() == 1)
    {
        qImg = QImage((const unsigned char*)(cvImg.data),
                      cvImg.cols, cvImg.rows,
                      cvImg.cols*cvImg.channels(),
                      QImage::Format_Indexed8);
    }
    else
    {
        qImg = QImage((const unsigned char*)(cvImg.data),
                      cvImg.cols, cvImg.rows,
                      cvImg.cols*cvImg.channels(),
                      QImage::Format_RGB888);
    }
    return qImg;
}