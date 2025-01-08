/*
 *  程序名：demo10.cpp，此程序演示开发框架中删除字符串左、右、两边指定字符的使用方法。
 *  作者：吴从周
*/
#include "../_public.h"
using namespace std;
using namespace idc;

int main()
{
    char str1[31];        // C风格的字符串。
    string str2;            // C++风格的字符串。

    strcpy(str1,"  西施  ");
    deletelchr(str1,' ');                // 删除str1左边的空格
    printf("str1=%s=\n",str1);    // 出输结果是str1=西施  =

    str2="  西施  ";
    deletelchr(str2,' ');
    cout << "str2=" << str2 << "=\n";

    strcpy(str1,"  西施  ");
    deleterchr(str1,' ');                // 删除str1左边的空格
    printf("str1=%s=\n",str1);    // 出输结果是str1=西施  =

    str2="  西施  ";
    deleterchr(str2,' ');
    cout << "str2=" << str2 << "=\n";

    strcpy(str1,"  西施  ");
    deletelrchr(str1,' ');               // 删除str1两边的空格
    printf("str1=%s=\n",str1);    // 出输结果是str1=西施=

    str2="  西施  ";
    deletelrchr(str2,' ');
    cout << "str2=" << str2 << "=\n";
}
