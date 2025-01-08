/*
 *  程序名：demo18.cpp，此程序演示开发框架正则表达示MatchStr函数的使用。
 *  作者：吴从周
*/
#include "../_public.h"
using namespace std;
using namespace idc;

int main()
{
    // 以下代码将输出yes。
    if (matchstr("_public.h","*.h,*.cpp")==true) printf("yes\n");
    else printf("no\n");

    // 以下代码将输出yes。
    if (matchstr("_public.h","*.H")==true) printf("yes\n");
    else printf("no\n");

    // 以下代码将输出no。
    if (matchstr("_public.h","*p*k*.h")==true) printf("yes\n");
    else printf("no\n");
}

