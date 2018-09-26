#ifndef PROGRAM_SERIAL_H
#define PROGRAM_SERIAL_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>
#include <iostream>
using namespace std;
// 串口设备信息结构
typedef struct tty_info_t
{
    int fd; // 串口设备ID
    pthread_mutex_t mt; // 线程同步互斥对象
    char name[24]; // 串口设备名称，例："/dev/ttyS0"
    struct termios ntm; // 新的串口设备选项
    struct termios otm; // 旧的串口设备选项
} TTY_INFO;

// 串口操作函数
TTY_INFO *readyTTY(int id);
int setTTYSpeed(TTY_INFO *ptty, int speed);
int setTTYParity(TTY_INFO *ptty,int databits,int parity,int stopbits);
int cleanTTY(TTY_INFO *ptty);
int sendnTTY(TTY_INFO *ptty,unsigned char *pbuf,int size);
int recvnTTY(TTY_INFO *ptty,unsigned char *pbuf,int size);
int lockTTY(TTY_INFO *ptty);
int unlockTTY(TTY_INFO *ptty);

#endif //PROGRAM_SERIAL_H

/*函数功能：
（1） 打开串口设备，调用函数setTTYSpeed（）；
（2） 设置串口读写的波特率，调用函数setTTYSpeed（）；
（3） 设置串口的属性，包括停止位、校验位、数据位等，调用函数setTTYParity（）；
（4） 向串口写入数据，调用函数sendnTTY（）；
（5） 从串口读出数据，调用函数recvnTTY（）；
（6） 操作完成后，需要调用函数cleanTTY（）来释放申请的串口信息接口；
（7）lockTTY（）和unlockTTY（）在多线程中使用。在读写操作的前后，锁定和释放串口资源。 */