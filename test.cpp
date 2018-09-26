#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
//#include <io.h>
#include <string.h>
#include <unistd.h>
int main1(void)
{
    int handle; unsigned char string[40] = {0xFF};
//    string[0] = 0xFF;
    unsigned char *content;
    content = string;
    int length, res;
    /* Create a file named "TEST.$$$" in the current directory and write a string to it. If "TEST.$$$" already exists, it will be overwritten. */
    if ((handle = open("../test.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE)) == -1)
    {
        printf("Error opening file.\n");
        exit(1);
    }
//    strcpy(string, "Hello, world!\n");
//    length = strlen(content);
    if ((res = write(handle,content, 1)) != 1)
    {
        printf("Error writing to the file.\n");
        exit(1);
    }
    printf("Wrote %d bytes to the file.\n", res);
    close(handle);
    return 0;
}