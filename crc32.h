#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFSIZE  1024*4

static unsigned int crc_table[256];

static void usage(void);
static void init_crc_table(void);
static unsigned int crc32(void* buffer, unsigned int size);
int test(int argc, char **argv);

/*
**初始化crc表,生成32位大小的crc表
**也可以直接定义出crc表,直接查表,
**但总共有256个,看着眼花,用生成的比较方便.
*/
static void init_crc_table(void)
{
    unsigned int c;
    unsigned int i, j;

    for (i = 0; i < 256; i++) {
        c = (unsigned int)i;
        for (j = 0; j < 8; j++) {
            if (c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc_table[i] = c;
    }
}

/*计算buffer的crc校验码*/
static unsigned int crc32(void* buffer, unsigned int size)
{
    unsigned int i,crc;
    unsigned char* pch;
    pch = (unsigned char*)buffer;
    crc = 0xFFFFFFFF;
//    if (size < 1)
//        return 0xffffffff;
    for (i = 0; i < size; i++) {
        crc = crc_table[(crc ^ *pch) & 0xff] ^ (crc >> 8);
        pch++;
    }
    return crc^0xFFFFFFFF ;
}


int test(int argc, char **argv)
{
    init_crc_table();
//    char *str="sdfdsfdsfdsfdsfdssdfdsfdsfdsfdsssssssssssssfdsfdsfds";
    unsigned char Num[9]={'1','2','3','4','5','6','7','8','9'};
    printf("%llX",crc32(Num,9));
    return 0;
}
