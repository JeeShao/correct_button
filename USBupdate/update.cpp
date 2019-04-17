#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
using namespace std;

string ch[2]={"usb","sd"}; //sda,sdb,sdc
int main(int argc,char **argv)
{
	int fd;
	char DEV[64];	//u盘等磁盘设备的设备文件路径
	char PATH[64];	//Update文件的路径
	char cmd[64];	//系统调用的命令
	int i=0;
	int j=0;	

	for(j=0;j<4;j++){	//最多支持4个分区
		for(i=0;i<2;i++){	//最多8个硬盘
			sprintf(PATH,"/media/%s/program_qt",ch[i].c_str()); //"/media/sda1/UpDate","/media/sda2/UpDate"...
			sprintf(DEV,"/media/sd%c%d",ch[i],j);  //对应的设备文件路径"/media/sda1","/media/sda2"...
			fd=open(PATH,O_RDONLY);    //打开文件全路径
            //(access(PATH,F_OK))==-1 //文件不存在
			if(fd==-1){    //判断文件是否存在
				continue;  //不存在继续扫描下一个分区
			}
			else{    //文件存在则跳出子循环
				printf("open device '%s'\n",PATH);
				break;
			}
		}
		if(fd!=-1)    //判断文件是否存在
			break;   //存在则跳出第二个for循环,不存在则继续下一个磁盘扫描
	}
	if(fd==-1){ //判断文件是否存在
		printf("can not find any u pan!\n ");  //这表示所有磁盘所有分区都没有UpDate文件
	}
	else{    //文件存在
		close(fd);  //关闭文件描述符
		//sprintf(cmd,"sh %s %s",PATH,DEV);  //设计执行脚本命令,例如"sh /media/sda1/UpDate /media/sda1"
		sprintf(cmd,"\\cp -rf %s /home/app/program_qt",PATH);  //设计执行脚本命令,例如"sh /media/sda1/UpDate /media/sda1"
		system(cmd);   //执行该脚本
		sprintf(cmd,"echo 123456|sudo chmod 777 /home/app/program_qt");	
		system(cmd); 
		system("echo 123456|sudo chown root /home/app/program_qt");
		system("reboot");
        //重启
	}
    system("cd /home/app");
    sprintf(cmd,"echo 123456|sudo ./program_qt");//执行程序
    system(cmd);
	return 0;
}
