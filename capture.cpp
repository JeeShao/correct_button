#include "capture.h"

Capture::Capture(int device=0):device(device){
    frame=NULL;
    currentFrame=NULL;
    showImg=NULL;
}

Capture::~Capture() {}

void Capture::open() {
    if(!capture.isOpened())
        capture.open(device);
}


void Capture::close() {
    if(capture.isOpened())
        capture.release();
}

int Capture::read(){
    capture>>frame;
    if(!frame.empty())
        return true;
    return false;
}

Mat Capture::getShowImg(){
    capture>>showImg;
    return showImg;
}

Mat Capture::getImg1(){
    return Q.front();
}

Mat Capture::getNextFrame(){
//    ;
//    if(!frame.empty())
//        currentFrame = frame.clone();
//    return read();
}

bool Capture::init(int exporsure,int gain){
    return exporsure+gain;
}
void Capture::setExposure(int value){
}
void Capture::setGain(int value){

}