#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
//-------------------------------------------------------------------------------------------------
typedef FILE* (*MY_FOPEN)(const char *path, const char *mode);
typedef FILE* (*MY_FREOPEN)(const char *path, const char *mode, FILE *stream);
typedef int (*MY_OPEN1)(const char *pathname, int flags);
typedef int (*MY_FCLOSE)(const char* pathname);
typedef int (*MY_CONNECT)(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen);
typedef ssize_t (*MY_WRITE)(int fd, const void *buf, size_t count);
//-------------------------------------------------------------------------------------------------
int sandbox_init() __attribute__((constructor));
int cpupretime;
int iolimit;
struct timeval iopretime;
long ioprewn;
int block_size;
const static long MEMLIMIT=5*1024*1024;
const static long IOWBS_LIMIT=1*1024*1024;
const static long SWITCH_TIME_INTERVAL=100*1;
//-------------------------------------------------------------------------------------------------

void SetIOLimit()
{
 ioprewn=0;
 gettimeofday(&iopretime,NULL);
}
//-------------------------------------------------------------------------------------------------

unsigned long GAP(struct timeval a,struct timeval b)
{
 if(a.tv_sec>b.tv_sec)
    return (a.tv_sec-b.tv_sec)*1000000+a.tv_usec-b.tv_usec;
 if(a.tv_sec==b.tv_sec&&a.tv_usec>b.tv_usec)
    return a.tv_usec-b.tv_usec;
 return 0;
}
//-------------------------------------------------------------------------------------------------

int is_file(int fd)
{
 struct stat buf;
 if(fstat(fd,&buf)==0&&(buf.st_mode&S_IFREG)==0)
    return -1;
 return 1;
}
//-------------------------------------------------------------------------------------------------

ssize_t write(int fd, const void *buf, size_t count)
{
 static MY_WRITE my_write=NULL;
 if(my_write==NULL)
   {
    void *handle=dlopen("libc.so.6", RTLD_LAZY);
    my_write=dlsym(handle,"write");
   }
 if(is_file(fd)<0)
    return my_write(fd,buf,count);
 static struct rusage ru;
 struct timeval ct;
 if(getrusage(RUSAGE_SELF,&ru)==0&&
    gettimeofday(&ct,NULL)==0)
   {
    unsigned long gap=GAP(ct,iopretime);
    if(gap>SWITCH_TIME_INTERVAL)
      {
       double a=(ru.ru_oublock-ioprewn)*block_size/4;
       double b=(double)(IOWBS_LIMIT)*gap/1000000;
       //char pbuf[256]; 
       //int n=sprintf(pbuf,"<%f,%f>\n",a,b);
       //my_write(0,pbuf,n+1);
       if(a>b)
         {
          double d=(double)(a-b)/b*gap;
          int st=d;
          //int n=sprintf(pbuf,"usleep %d\n",st);
          //my_write(0,pbuf,n+1);
          usleep(st);
         }
       iopretime=ct;
       ioprewn=ru.ru_oublock;
      }
   }
 return my_write(fd,buf,count);
}
//-------------------------------------------------------------------------------------------------

void SetCPULimit(int reset)
{
 struct rlimit rl;
 if(reset<0)
   {
    rl.rlim_cur=1;
    rl.rlim_max=RLIM_INFINITY;
   }
 else
   {
    getrlimit(RLIMIT_CPU,&rl);
    rl.rlim_cur+=1;
   }
 printf("set cpu limit [%lu,%lu]\n",rl.rlim_cur,rl.rlim_max);
 setrlimit(RLIMIT_CPU,&rl);
 cpupretime=time(NULL);
}
//-------------------------------------------------------------------------------------------------

void SetMemLimit()
{
 struct rlimit rl;
 rl.rlim_cur=MEMLIMIT;
 rl.rlim_max=MEMLIMIT;
 setrlimit(RLIMIT_AS,&rl);
 printf("set mem limit [%lu,%lu]\n",rl.rlim_cur,rl.rlim_max);
}
//-------------------------------------------------------------------------------------------------

void SIGXCPU_handler(int a)
{
 printf("SIGXCPU_handler\n");
 int currenttime=time(NULL);
 int gap=currenttime-cpupretime;
 if(gap>2)
   {
    printf("gap>2\n");
    SetCPULimit(1);
    return;
   }  
 else 
   {
    printf("run usleep\n");
    usleep(2000000);
   }
}
//-------------------------------------------------------------------------------------------------

int sandbox_init()
{
 struct stat fi;
 stat("/", &fi);
 block_size=fi.st_blksize;
 printf("sandbox init %d\n",block_size);
 //SetCPULimit(-1);
 //signal(SIGXCPU,SIGXCPU_handler);
 //SetMemLimit();
 SetIOLimit();
 return 1;
}
//-------------------------------------------------------------------------------------------------
