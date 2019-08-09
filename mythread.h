#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QDebug>
#include <QWaitCondition>
#include <QMutex>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <math.h>
#include <pthread.h>
#include "serial.h"
#include "common.h"
#include "capture.h"
#include "camera.h"
using namespace cv;

extern QMutex mutex;
extern QWaitCondition finishCond;//完成拍照 条件锁
extern volatile int FINISH_TAKE_PHOTO; //是否完成拍照 0否 1是 -1掉线
extern Mat PHOTO;//获取到的拍照图像

class MyThread:public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = 0);
    ~MyThread();
    int generate_temp();
    int match();
    bool takePhoto();
    void init_param(double r);

public:
    static unsigned char DATA_MSG[5];//通知
    static unsigned char DATA_ANGLE[6];//旋转角度
    static unsigned char DATA_REC[8];//接受串口信息

protected:
    void run();

signals:
    void takePhoto_signal();
    void updateStatus(string status);
    void updateAngle(string angle);
    void updateExposure(int val);
    void updateGain(int val);
    void showInitImg(Mat img);//显示校正图像
    void cameraLoss();

public slots:
    void lossCamera();
private:
    TTY_INFO *ptty;  //串口

private:
    void printData(unsigned char *arr,int size);
    inline int initPtty();
    void sendCapLoseMsg();
    void setParams();
};

#endif