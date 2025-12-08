#include "kernel.h"
#include "time.h"

struct timeval {
    time_t tv_sec;
    suseconds_t tv_usec;
};

int gettimeofday (struct timeval *restrict tp, void * restrict tzp)
{
    /*tzpは無視*/
    uint32_t us;
    uint64_t s;
    SYSTIM ctime;
    get_tim(&ctime);
    us=ctime % 1000000;
    s = ( ctime-us )/1000000; 
    tp->tv_sec = s;
    tp->tv_usec = us;
    return 0;
}