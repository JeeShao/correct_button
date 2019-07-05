#include <QtCore>
#include <QMainWindow>
#include <QLabel>
#include <QApplication>
#include <QGuiApplication>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QThread>
#include <QString>
#include <QDesktopWidget>
#include <QTimer>
#include <string>
#include <iostream>
#include <unistd.h>
#include "mythread.h"

using namespace std;

class MainWindow : public QMainWindow
{
    Q_OBJECT//信号与槽

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setupUi(QMainWindow *MainWindow);
    void retranslateUi(QMainWindow *MainWindow);
    QImage mat2QImage(Mat cvImg); //Mat转QImage
public:
    int screen_w;//屏幕宽
    int screen_h;//屏幕高
    QWidget *centralwidget;
    QLabel *label_title;
    QLabel *label_frame;
    QLabel *label_angle;
    QLabel *label_angle_val;
    QLabel *label_status;
    QLabel *label_status_val;
    QLabel *label_logo;
    QFont font1;


public slots:
    void show_frame();
    void dealTakePhoto();
    void setStatus(string status);
    void setAngle(string angle);
    void setExposure(int val);
    void setGain(int val);
    void setShowInitImg(Mat img);

signals:
    void cameraLoss();


private:
    //××××××××××更改相机驱动××××××××
    //Capture *cap;
    Camera *cap;
    //××××××××××**********××××××××
    QTimer *timer1;
    Mat image;
    MyThread *mythread;
    int img_no;
    int width=1280;
    int height=1024;
    QMatrix matrix;
    Point start1;//中心线
    Point end1 ;
    Point start2;
    Point end2;

    bool CapInit;
};


