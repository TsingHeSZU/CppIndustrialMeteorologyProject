#include "/CppIndustrialMeteorologyProject/public/_public.h"
using namespace idc;

// 进程心跳机制
Cpactive pactive;

// 程序退出和信号 2、15 的处理函数
void EXIT(int sig);

int main(int argc, char* argv[]) {
    // 程序帮助文档
    if (argc != 4)
    {
        printf("\n");
        printf("Using: \n/CppIndustrialMeteorologyProject/tools/bin/deletefiles pathname matchstr timeout\n\n");

        printf("Examples: \n/CppIndustrialMeteorologyProject/tools/bin/deletefiles /tmp/idc/surfdata \"*.xml,*.json\" 0.01\n");
        cout << R"(/CppIndustrialMeteorologyProject/tools/bin/deletefiles /log/idc "*.log.20*" 0.02)" << endl;
        printf("/CppIndustrialMeteorologyProject/tools/bin/procctl 300 /CppIndustrialMeteorologyProject/tools/bin/deletefiles /log/idc \"*.log.20*\" 0.02\n");
        printf("/CppIndustrialMeteorologyProject/tools/bin/procctl 300 /CppIndustrialMeteorologyProject/tools/bin/deletefiles /tmp/idc/surfdata \"*.xml,*.json\" 0.01\n\n");

        printf("这是一个工具程序, 用于删除历史的数据文件或日志文件。\n");
        printf("本程序把 pathname 目录及子目录中 timeout 天之前的匹配 matchstr 文件全部删除, timeout 可以是小数。\n");
        printf("本程序不写日志文件，也不会在控制台输出任何信息。\n\n");

        return -1;
    }

    // 忽略全部的信号和关闭 I/O，设置信号处理函数
    closeioandsignal(true);     // 在开发测试阶段，这行代码可以注释，方便显示调试信息
    signal(2, EXIT);
    signal(15, EXIT);

    pactive.addProcInfo(30, "deletefiles");     // 心跳超时时间为 30s，把当前进程的心跳加入共享内存

    // 获取被定义为历史数据文件的时间点
    string str_timeout = ltime1("yyyymmddhh24miss", 0 - (int)(atof(argv[3]) * 24 * 60 * 60));

    // 打开目录
    cdir dir;
    if (dir.opendir(argv[1], argv[2], 10000, true, false) == false) {
        printf("dir.opendir(%s) failed.\n", argv[1]);
        return -1;
    }

    // 遍历目录中的文件，如果是历史数据文件，删除它
    while (dir.readdir() == true) {
        // 把文件的时间与历史文件的时间点比较，如果更早，就需要删除
        if (dir.m_mtime < str_timeout) {
            if (remove(dir.m_ffilename.c_str()) == 0) {   // m_ffilename 表示绝对路径文件名
                printf("remove(%s) OK.\n", dir.m_ffilename.c_str());
            }
            else {
                printf("remove(%s) failed.\n", dir.m_ffilename.c_str());
            }
        }
    }

    return 0;
}


void EXIT(int sig) {
    printf("程序退出, sig = %d\n", sig);
    exit(0);
}