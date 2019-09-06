#include "mainwindow.h"
int main(int argc, char *argv[])
{
	if(init_sys())
		return -1;
    qRegisterMetaType<string>("string");//注册string
    qRegisterMetaType<Mat>("Mat");//注册Mat

    QApplication a(argc, argv);
    MainWindow w;
    QGraphicsScene *scene = new QGraphicsScene;
    QGraphicsProxyWidget *g = scene->addWidget(&w);
    g->setRotation(-90);//-90
    QGraphicsView *view = new QGraphicsView(scene);
    view->showFullScreen();
//    view->show();

    return a.exec();
}
