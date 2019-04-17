#include <stdio.h>
#include <iostream>
#include <fstream>
#include <linux/hdreg.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

using namespace std;
string FILE_PATH = "sysdisk.dat";
char *ENDTIME = "20190710"; //截止时间

/*
 * 函数功能：将字符转换成对应的十进制数
 */
int char2int( char input )
{
    return input>64?(input-55):(input-48);
}

/*
 * 函数功能：将十六进制数值转换为对应字符
 */
int int2char( char input )
{
    return input>9?(input+55):(input+48);
}

/*
 * 函数功能：对两个8位十六进制字串进行异或
 * 返回值：8位十六进制异或结果
 */
void hexStrXor( char * HexStr1, char * HexStr2, char * HexStr )
{
    int i, iHexStr1Len, iHexStr2Len, iHexStrLenLow, iHexStrLenGap;
    //转换成大写并求长度
    iHexStr1Len = strlen( HexStr1 );
    iHexStr2Len = strlen( HexStr2 );
    //获取最小的长度
    iHexStrLenLow = iHexStr1Len<iHexStr2Len?iHexStr1Len:iHexStr2Len;
    //获取长度差值
    iHexStrLenGap = abs( iHexStr1Len-iHexStr2Len );
    //两个十六进制的字符串进行异或
    for( i=0; i<iHexStrLenLow; i++ )
    {
        *(HexStr+i) = char2int( HexStr1[i] ) ^ char2int( HexStr2[i] );
        *(HexStr+i) = int2char( *(HexStr+i) );
    }
    if( iHexStr1Len>iHexStr2Len )
        memcpy( HexStr+i, HexStr1+i, iHexStrLenGap );
    else if( iHexStr1Len<iHexStr2Len )
        memcpy( HexStr+i, HexStr2+i, iHexStrLenGap );
    *( HexStr+iHexStrLenLow+iHexStrLenGap ) = 0x00;
}

/*
 * 函数功能：获取8位当前日期
 */
void getDate(char *dateNow) {
    string datetime;
    time_t now = time(0);
    tm *ltm = localtime(&now);

    string mon = (1+ltm->tm_mon) <10? '0'+to_string(1+ltm->tm_mon):to_string(1+ltm->tm_mon);
    string day = ltm->tm_mday <10? '0'+to_string(ltm->tm_mday):to_string(ltm->tm_mday);

    datetime = to_string(1900+ltm->tm_year) + mon+ day;
    sprintf(dateNow,"%s", (char*)datetime.c_str());
}


/*
 * 函数功能：获取硬盘8位十六进制序列号
 * 返回值：0(成功) -1(失败)
 * 说明：序列号中包含非十六进制字符时 将其对10取模后转换成0-9的数字
 */
static int getdiskid (char *hardc)
{
    int fd;
    char serialNo[8];//8位
    struct hd_driveid hid;
    fd = open ("/dev/sda", O_RDONLY);
    if (fd < 0)
    {
        return -1;
    }
    if (ioctl (fd, HDIO_GET_IDENTITY, &hid) < 0)
    {
        return -1;
    }

    close(fd);
    for(int i=12;i<20;i++){
        serialNo[i-12] = hid.serial_no[i]<71? hid.serial_no[i]:(hid.serial_no[i]%10+48);
    }
    // serialNo = string((char*)hid.serial_no).substr(12,8).c_str();
    sprintf(hardc,"%s", serialNo);
    return 0;
}

int main(void)
{
    char hardseri[8];//8位序列号
    char dateNow[8];//8位当前日期
    char xorNowRes[8];//8位异或结果
    char xorEndRes[8];//8位异或结果
    if(getdiskid(hardseri)==-1){
     cout<<"No"<<endl;
     return -1;
    }
    getDate(dateNow);
    hexStrXor(hardseri,dateNow,xorNowRes);
    hexStrXor(hardseri,ENDTIME,xorEndRes);
    printf("序列号:%s\n",hardseri);
    printf("当前日期:%s\n",dateNow);
    printf("当前异或:%s\n",xorNowRes);
    printf("截止异或:%s\n",xorEndRes);
    fstream file;
    try{
        file.open(FILE_PATH.c_str(),ios::out);
        file.clear();//先清空文件
    }catch (exception &e){
        cerr<<e.what()<<endl;
        cerr<<"文件打开失败!"<<endl;
        exit(0);
    }
    file << "lastserial="<<xorNowRes<<endl;
    file << "endserial="<<xorEndRes<<endl;
    cout<<"写入成功"<<endl;
    file.close();
    return 0;
}