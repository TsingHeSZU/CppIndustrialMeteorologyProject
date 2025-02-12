#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
    调度模块
*/

int main(int argc, char* argv[]) {
    if (argc < 3)
    {
        printf("Using: ./procctl timetvl program argv ...\n");
        printf("Example: /CppIndustrialMeteorologyProject/tools/bin/procctl 10 /usr/bin/tar zcvf /tmp/tmp.tgz /usr/include\n");
        printf("Example: /CppIndustrialMeteorologyProject/tools/bin/procctl 60 /CppIndustrialMeteorologyProject/idc/bin/crtsurfdata /CppIndustrialMeteorologyProject/idc/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log csv,xml,json\n");

        printf("本程序是服务程序的调度程序, 周期性启动服务程序或 shell 脚本。\n");
        printf("timetvl: 运行周期，单位：秒。\n");
        printf("    - 被调度的程序运行结束后, 在timetvl秒后会被 procctl 重新启动。\n");
        printf("    - 如果被调度的程序是周期性的任务, timetvl设置为运行周期。\n");
        printf("    - 如果被调度的程序是常驻内存的服务程序, timetvl 设置小于 5 秒。\n");
        printf("program: 被调度的程序名, 必须使用全路径。\n");
        printf("...: 被调度的程序的参数。\n");
        printf("注意: 本程序不会被 kill 杀死, 但可以用 kill -9 强行杀死。\n\n");

        return -1;
    }

    /*
        关闭信号和 I/O，调度程序不希望被打扰
        - 为了防止调度程序被误杀，不处理退出信号
        - 如果关闭了 I/O，将影响被调度的程序（也会关闭 I/O）
    */
    for (int i = 0; i < 64; ++i) {
        signal(i, SIG_IGN);
        close(i);   // 关闭文件描述符 0 ~ 63
    }

    // 创建子进程，父进程退出，让程序运行在后台，由系统 1 号进程托管，不受 shell 的控制
    if (fork() != 0) {
        exit(0);    // 父进程作为前台进程，退出
    }

    // 启用 SIGCHLD 信号，让父进程可以 wait 子进程退出的状态，回收子进程资源
    signal(SIGCHLD, SIG_DFL);   // SIG_DFL 表示默认信号动作

    // 定义一个和 argv 一样大的指针数组，存放被调度程序名及其参数
    char* pargv[argc];
    for (int i = 2; i < argc; ++i) {
        pargv[i - 2] = argv[i];
    }
    pargv[argc - 2] = NULL; // 空表示参数已结束

    while (true) {
        if (fork() == 0) {
            // 子进程运行被调度的程序
            int ret = execv(argv[2], pargv);
            if (ret == -1) {
                exit(0);    // 如果调度失败，子进程退出
            }
        }
        else {
            // 父进程等待子进程终止（被调度的程序运行结束）
            int status;
            wait(&status);      // wait() 函数会阻塞，直到被调度的程序终止
            sleep(atoi(argv[1]));   // 休眠 timetvl 秒，然后回到循环
        }
    }

    return 0;
}