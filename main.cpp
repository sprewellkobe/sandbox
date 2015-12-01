#include<stdio.h>
#include<malloc.h>
#include<stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>
#include <vector>
#include <sys/time.h>
#include <sys/resource.h>
using namespace std;
//-------------------------------------------------------------------------------------------------
unsigned long long FILE_MAX_SIZE=70*1024*1024;
const static int WRITE_SIZE=800000;
unsigned long long wrote_bytes=0;
//-------------------------------------------------------------------------------------------------
typedef FILE* (*MY_FOPEN)(const char *path, const char *mode);
typedef FILE* (*MY_FREOPEN)(const char *path, const char *mode, FILE *stream);
//-------------------------------------------------------------------------------------------------

void IOLimitTest()
{
 char* filename=(char*)("/tmp/kobe.dat");
 int fd=open(filename,O_CREAT|O_WRONLY|O_SYNC);
 if(fd<=0)
   {
    printf("open failed\n");
    return;
   }
 unsigned long long fs=0;
 for(int i=0;i<20000;i++)
    {
     char buffer[WRITE_SIZE];
     int rv=write(fd,buffer,WRITE_SIZE);
     if(rv==WRITE_SIZE)
       {
        wrote_bytes+=WRITE_SIZE;
        fs+=WRITE_SIZE;
        printf("wrote %llu\n",fs);
       }
     else
        printf("errno %d,%d\n",rv,errno);
     if(fs>FILE_MAX_SIZE)
       {
        lseek(fd,0,SEEK_SET);
        fs=0;
       }
    }
 if(fd>0)
    close(fd);
}
//-------------------------------------------------------------------------------------------------

void CPUTest()
{
 for(;;)
     ;
}
//-------------------------------------------------------------------------------------------------

void MemTest()
{
 vector<void*> pa;
 long initsize=1024;
 for(;;)
    {
     void* p=malloc(initsize);
     if(p==NULL)
       {
        printf("malloc failed, restart......\n");
        sleep(1);
        if(errno==ENOMEM)
           initsize=1024;
        continue;
       }
     printf("malloced %lu bytes\n",initsize);
     initsize*=2;
     free(p); 
    }
}
//-------------------------------------------------------------------------------------------------

int main(void)
{
 //CPUTest();
 //MemTest();
 IOLimitTest();
 return 0;
}
//-------------------------------------------------------------------------------------------------
