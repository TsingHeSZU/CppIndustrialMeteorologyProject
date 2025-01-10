#include "/CppIndustrialMeteorologyProject/public/_public.h"
using namespace idc;
/*
    守护程序：检查共享内存中进程的心跳，如果超时，则终止进程
*/

int main(int argc, char* argv[]) {
    // 程序运行提示
    if (argc != 2)
    {
        printf("\n");
        printf("Using: ./checkproc logfilename\n");

        printf("Example: /CppIndustrialMeteorologyProject/tools/bin/procctl 10 /CppIndustrialMeteorologyProject/tools/bin/checkproc /tmp/log/checkproc.log\n\n");

        printf("本程序用于检查后台服务程序是否超时，如果已超时，就终止它\n");
        printf("注意: \n");
        printf("1. 本程序由procctl启动, 运行周期建议为 10 秒\n");
        printf("2. 为了避免被普通用户误杀, 本程序应该用 root 用户启动\n");
        printf("3. 如果要停止本程序, 只能用 killall -9 终止\n\n");
        return 0;
    }

    // 忽略全部的信号和 IO，不处理程序的退出信号，只能用 kill -9 终止
    closeioandsignal(true);

    // 1. 打开日志文件
    clogfile logfile;
    if (!logfile.open(argv[1])) {
        printf("logfile.open(%s) failed.\n", argv[1]);
        return -1;
    }

    // 创建 / 获取共享内存，键值为 SHMKEYP，大小为 MAXNUMP 个 StProcInfo 结构体的大小    
    int shm_id = -1;
    shm_id = shmget(SHMKEYP, MAXNUMP * sizeof(StProcInfo), 0666 | IPC_CREAT);
    if (shm_id == -1) {
        logfile.write("创建 / 获取共享内存(%x)失败。\n", SHMKEYP);
        return false;
    }

    // 2. 将共享内存连接到当前进程的地址空间
    StProcInfo* shm_addr = (StProcInfo*)shmat(shm_id, nullptr, 0);

    // 3. 遍历共享内存中全部的记录，如果进程已超时，终止它
    for (int i = 0; i < MAXNUMP; ++i) {
        // 当前共享内存第 i 个位置的 pid == 0 (共享内存在创建时，默认初始化为 0)，表示空记录，continue
        if ((shm_addr + i)->pid == 0) {
            continue;
        }

        /*
            如果存在异常退出的服务进程，共享内存中残留了其心跳信息
            向后台进程发送信号 0，判断它是否还存在，如果不存在，从共享内存中删除该记录
        */
        int i_ret = kill(shm_addr[i].pid, 0);
        if (i_ret == -1) {
            logfile.write("进程 pid = %d(%s) 已经不存在\n", shm_addr[i].pid, shm_addr[i].pname);
            memset(shm_addr + i, 0, sizeof(StProcInfo));    // 从共享内存中删除该记录
            continue;
        }

        // 判断进程的心跳是否超时了，如果超时了，就终止它
        time_t now_time = time(0);   // 获取当前时间

        // 如果进程未超时，continue
        if (now_time - shm_addr[i].atime < shm_addr[i].timeout) {
            continue;
        }

        // 把进程的结构体备份出来，不能直接用共享内存的值，否则会误杀本守护进程
        StProcInfo tmp = shm_addr[i];

        // 已超时
        logfile.write("进程 pid = %d(%s) 已经超时\n", tmp.pid, tmp.pname);

        // 发送信号 15，尝试正常终止已超时的后台进程
        kill(tmp.pid, 15);

        // 每隔 1 秒判断一次后台进程是否存在，累计 5 秒，让 OS 可以有时间回收后台进程资源
        for (int j = 0;j < 5;++j) {
            sleep(1);
            i_ret = kill(tmp.pid, 0);   // 向后台进程发送信号 0，判断它是否还存在
            if (i_ret == -1) {
                break;  // 进程已退出
            }
        }

        //  后台进程正常终止
        if (i_ret == -1) {
            logfile.write("进程 pid = %d(%s) 已经正常终止\n", tmp.pid, tmp.pname);
        }
        else {
            // 后台进程没有正常终止，强行终止
            kill(tmp.pid, 9);
            logfile.write("进程 pid = %d(%s) 已经强制终止\n", tmp.pid, tmp.pname);

            // 从共享内存中删除已经超时进程的心跳记录
            memset(shm_addr + i, 0, sizeof(StProcInfo));
        }
    }

    // 4. 把共享内存从当前进程中分离
    shmdt(shm_addr);

    return 0;
}