#include <math.h>
#include <pthread.h>
#include "rect_image.h"
#include "match.h"
#include "serial.h"
#include "JHCap.h"
#include "bmpfile.h"
#include "common.h"
#include "camera.h"


int main() {
    Camera cap(1280,1024);
    cap.init();
    Mat img;
    int i=1;
    clock_t start,finish;
    namedWindow("a",0);
    resizeWindow("a",640,512);

    while(cap.read().empty()){
        cout<<'a';
    }
    start = clock();

//    total_time = (double)((finish-start)*1000/CLOCKS_PER_SEC);//ms
    while((double)((clock()-start)*1000/CLOCKS_PER_SEC)<=1000){
        img = cap.read();
        i++;
//        imshow("a",img);
//        imwrite(to_string(i++)+".jpg",img);
    }
    cout<<(double)((clock()-start)*1000/CLOCKS_PER_SEC)<<"ms"<<endl;
    cout<<i-1<<"张";
    return 0;
}

