#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QMainWindow>
#include <QLabel>
#include <QMenuBar>
#include <QApplication>
#include <QGuiApplication>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QThread>
#include <QString>
#include <QDesktopWidget>
#include "Mylabel.h"
#include <math.h>
#include <pthread.h>
#include "serial.h"
#include "JHCap.h"
#include "common.h"
//#include "camera.h"
#include "camera.h"


//驱动相机
//Camera capture(width,height);

class MainWindow : public QMainWindow,public QThread
{
//Q_OBJECT

public:
    int screen_w;//屏幕宽
    int screen_h;//屏幕高
    QWidget *centralwidget;
    QLabel *label_title;
    Mylabel *label_frame;
    QLabel *label_angle;
    QLabel *label_angle_val;
    QLabel *label_status;
    QLabel *label_status_val;
    QLabel *label_logo;
    QFont font1;
    static MainWindow* mainwindow;

    static unsigned char DATA_MSG[5];//通知
    static unsigned char DATA_ANGLE[6];//旋转角度
    static unsigned char DATA_REC[8];//接受串口信息

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setupUi(QMainWindow *MainWindow);
    void retranslateUi(QMainWindow *MainWindow);
    QImage mat2QImage(Mat mat);
    int generate_temp();
    int match();
    void run();
    int startFun();
    void showImg(Mat img);

private:
    Camera *cap;
    TTY_INFO *ptty;  //串口
    int width=1280/*376*/;
    int height=1024/*240*/;
    int len=0;
    QImage Qimage;//旋转
    QMatrix matrix;
    //中心线
    Point start1;
    Point end1 ;
    Point start2;
    Point end2;

private:
    void printData(unsigned char *arr,int size);
    void init_param(double r);
    void updateStatus();
    void updateAngle();
    inline int initPtty();
    void sendCapLoseMsg();
    void setParams();
};

#endif