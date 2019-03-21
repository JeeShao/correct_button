#pragma once
#include <QtCore>
#include <QLabel>


/*******************************************
—————摄像头框类————————
********************************************/
class Mylabel :public QLabel
{
public:
    explicit Mylabel(QWidget *parent = 0);
    void setRect(int x1, int y1, int x2, int y2);
private:
    int x1, y1, x2, y2;
};

