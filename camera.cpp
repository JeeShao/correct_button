#include "camera.h"

Camera::Camera(int width=1280,int height=1024) :width(width),height(height)
{
    image = cvCreateImage(cvSize(width, height), 8, 3);
    frame=NULL;
    len=0;
}

Camera::~Camera() {}

//初始化相机
bool Camera::init(int exporsure,int gain){
    int count=0;
    CameraGetCount(&count);
//    printf("Camera count: %d\n", count);
    if(count<1)
        return false;
    CameraInit(0);
    CameraSetGamma(0,1.33);//咖马值
    CameraSetContrast(0,1);//对比度
    CameraSetSaturation(0,1.18);//饱和度
    CameraSetBlackLevel(0,10);//黑电平
    CameraSetGain(0, gain);//增益150
    CameraSetExposure(0, exporsure);//曝光800
//    CameraSetOption(0,CAMERA_IMAGE_RGB24);//图像格式
    CameraSetSnapMode(0, CAMERA_SNAP_CONTINUATION);
    CameraSetResolution(0, 2, &width, &height);
    CameraGetImageBufferSize(0, &len, CAMERA_IMAGE_BMP);
    return true;
}


void Camera::close() {
    CameraFree(0);
}

int Camera::read() {
    if (CameraQueryImage(0, (unsigned char *) image->imageData, &len,CAMERA_IMAGE_BMP) == API_OK) {
        frame = cvarrToMat(image);
        return true;
    }
   return false;
}

Mat Camera::getImg(){
    return this->frame;
}

void Camera::setExposure(int value) {
    CameraSetExposure(0, value);
}

void Camera::setGain(int value) {
    CameraSetGain(0, value);
}