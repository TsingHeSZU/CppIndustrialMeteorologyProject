/*
 *  程序名：demo26.cpp，此程序演示开发框架中整数表示的时间和字符串表示的时间之间的转换。
 *  作者：吴从周
*/
#include "../_public.h"
using namespace std;
using namespace idc;

int main()
{
    string strtime;
    strtime="2020-01-01 12:35:22";

    time_t ttime;
    ttime=strtotime(strtime);        // 转换为整数的时间
    printf("ttime=%ld\n",ttime);    // 输出ttime=1577853322
  
    char s1[20];                             // C风格的字符串。
    timetostr(ttime,s1,"yyyy-mm-dd hh24:mi:ss");  // 转换为字符串的时间
    cout << "s1=" << s1 << endl;

    string s2;                               // C++风格的字符串。
    timetostr(ttime,s2,"yyyy-mm-dd hh24:mi:ss");  // 转换为字符串的时间
    cout << "s2=" << s2 << endl;
}

