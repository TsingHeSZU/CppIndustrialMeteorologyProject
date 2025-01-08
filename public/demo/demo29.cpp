/*
 *  程序名：demo29.cpp，此程序演示开发框架中的ctimer类（计时器）的用法。
 *  作者：吴从周
*/
#include "../_public.h"
using namespace std;
using namespace idc;

int main()
{
    ctimer timer;

    printf("elapsed=%lf\n",timer.elapsed());
    sleep(1);
    printf("elapsed=%lf\n",timer.elapsed());
    sleep(1);
    printf("elapsed=%lf\n",timer.elapsed());
    usleep(1000);
    printf("elapsed=%lf\n",timer.elapsed());
    usleep(100);
    printf("elapsed=%lf\n",timer.elapsed());
    sleep(10);
    printf("elapsed=%lf\n",timer.elapsed());
}

