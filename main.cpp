#include "mainwindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QGraphicsScene *scene = new QGraphicsScene;
    QGraphicsProxyWidget *g = scene->addWidget(&w);
    g->setRotation(0);//-90
    QGraphicsView *view = new QGraphicsView(scene);

    view->show();
    w.start();//启动线程
    return a.exec();
}

//获取分辨率
//int currentScreenWid = QApplication::desktop()->width();
//int currentScreenHei = QApplication::desktop()->height();

//Form *form = new Form; //项目主窗口
//
//QGraphicsScene *scene = new QGraphicsScene;
//QGraphicsProxyWidget *w = scene->addWidget(form);
//w->setRotation(90);
//
//QGraphicsView *view = new QGraphicsView(scene);
//view->resize(810, 610);
//view->show();