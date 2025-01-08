/*
 *  程序名：demo34.cpp，此程序演示开发框架中采用cdir类获取某目录及其子目录中的文件列表信息。
 *  作者：吴从周
*/
#include "../_public.h"
using namespace std;
using namespace idc;

int main(int argc,char *argv[])
{
    if (argc != 3) 
    { 
        printf("Using:./demo34 pathname matchstr\n");
        printf("Sample:./demo34 /project \"*.h,*.cpp\"\n");
        return -1;
    }

    cdir dir;       // 创建读取目录的对象。

    if (dir.opendir(argv[1],argv[2],100,false,true)==false)             // 打开目录，获取目录中文件的列表。
    {   
        printf("dir.opendir(%s) failed.\n",argv[1]); return -1; 
    }

    while(dir.readdir()==true)        // 遍历文件列表。
    {
        cout << "filename=" << dir.m_ffilename << ",mtime=" << dir.m_mtime << ",size=" << dir.m_filesize << endl;
    }
}
