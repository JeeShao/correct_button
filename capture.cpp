//
// Created by app on 19-1-16.
//

#include "capture.h"

Capture::Capture(int device=0):device(device){
    frame=NULL;
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

Mat Capture::read(){
    capture>>frame;
    if(Q.size()>=2){
//        img=Q.front().clone();
        Q.pop();
//        std::cout<<"22"<<std::endl;
    }
    Q.push(frame);

    return this->frame;
}

Mat Capture::getImg(){
    return Q.front();
}

bool Capture::init(int exporsure,int gain){
    return exporsure+gain;
}
void Capture::setExposure(int value){
}
void Capture::setGain(int value){

}