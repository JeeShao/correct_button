#include <math.h>
#include <pthread.h>
#include "rect_image.h"
#include "match.h"
#include "serial.h"
#include "JHCap.h"
#include "bmpfile.h"
#include "common.h"



TTY_INFO *ptty;  //串口
unsigned char DATA_MSG[] = {0xDE,0xA9,00,0xFF,0xFF};  //通知
unsigned char DATA_ANGLE[] = {0xDE,0xA1,01,00,0xFF,0xFF}; //旋转角度
unsigned char DATA_WIDTH[] = {0xDE,0xFC,00,0xFF,0xFF}; //纽扣宽度
unsigned char DATA_REC[8] = {}; //接受串口信息

int main() {

//    ptty = readyTTY(0);
//    int nbyte;
//    unsigned char cc[16];
//
//    if (ptty == NULL) {
//        printf("readyTTY(0) error\n");
//        return -1;
//    }
//
//    lockTTY(ptty);
//    if (setTTYSpeed(ptty, 9600) > 0) {  //设置波特率
//        printf("setTTYSpeed() error\n");
//        return -1;
//    }
//    if (setTTYParity(ptty, 8, 'N', 1) > 0) { //设置通讯格式
//        printf("setTTYParity() error\n");
//        return -1;
//    }
////    fcntl(ptty->fd, F_SETFL, FNDELAY);//非阻塞
//    fcntl(ptty->fd, F_SETFL, 0);//阻塞
//
//    while (1) {
////        sendnTTY(ptty,DATA_MSG,5);
//
//
//        if (recvnTTY(ptty, DATA_REC, 4) == 4) {
//            for(int i=0;i<4;i++){
//                printf("%02X\n",DATA_REC[i]);
//            }
//            cout<<"data: "<<DATA_REC[0]<<'-'<<DATA_REC[1]<<'-'<<DATA_REC[2]<<'-'<<DATA_REC[3]<<endl;
//            memset(DATA_REC,0,8);
//            sleep(1);
//        }
//
//    }

    Mat src, src_gray;

    /// Read the image
    src = imread( "circle1.jpg", 1 );

    if( !src.data )
    { return -1; }

    /// Convert it to gray
    cvtColor( src, src_gray, CV_BGR2GRAY );

    /// Reduce the noise so we avoid false circle detection
    GaussianBlur( src_gray, src_gray, Size(9, 9), 2, 2 );

    vector<Vec3f> circles;

    /// Apply the Hough Transform to find the circles
    HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, src_gray.rows/8, 100, 50, src_gray.rows/8, src_gray.rows/3 );

    /// Draw the circles detected
    cout<<"检测到"<<circles.size()<<"个圆"<<endl;
    cout<<"center:"<<"("<<circles[0][0]<<','<<circles[0][1]<<')'<<"R:"<<circles[0][2]<<endl;

    for( size_t i = 0; i < circles.size(); i++ )
    {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        // circle center
        circle( src, center, 3, Scalar(0,255,0), -1, 8, 0 );
        // circle outline
        circle( src, center, radius, Scalar(0,0,255), 3, 8, 0 );
    }

    /// Show your results
    namedWindow( "Hough Circle Transform Demo", 0 );
    resizeWindow("Hough Circle Transform Demo",src.cols/2,src.rows/2);
    imshow( "Hough Circle Transform Demo", src );

    waitKey(0);
}

