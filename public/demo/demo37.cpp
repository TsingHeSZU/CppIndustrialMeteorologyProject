/*
 *  程序名：demo37.cpp，此程序演示开发框架中采用cifile类从文本文件中读取数据。
 *  作者：吴从周
*/
#include "../_public.h"
using namespace std;
using namespace idc;

int main()
{
    cifile ifile;

    // 打开文件。
    if (ifile.open("/tmp/data/girl.xml")==false)
    {
        printf("ofile.open(/tmp/data/girl.xml) failed.\n"); return -1;
    }

    string strline;   // 存放从文本文件中读取的一行。

    while (true)
    {
        // 从文件中读取一行。
        if (ifile.readline(strline,"<endl/>")==false) break;

        cout << "=" << strline << "=\n";
    }

    // ifile.closeandremove();     // 关闭并删除文件。
    ifile.close();                       // 关闭文件。
}