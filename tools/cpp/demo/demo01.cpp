#include "/CppIndustrialMeteorologyProject/public/_public.h"
using namespace idc;


/*
    进程心跳的实现，功能已经封装到了 _public.h 中
*/

// // 进程心跳信息的结构体
// struct StProcInfo {
//     int pid = 0;        // 进程 id
//     char pname[51] = { 0 };     // 进程名称，可以为空
//     int timeout = 0;        // 超时时间，单位：秒
//     time_t atime = 0;       // 最后一次心跳的时间，用整数表示

//     StProcInfo() = default;   // 有了自定义的构造函数，编译器将不提供默认构造函数，所以启用默认构造函数
//     StProcInfo(const int in_pid, const string& in_pname, const int in_timeout, const time_t in_atime)
//         :pid(in_pid), timeout(in_timeout), atime(in_atime) {
//         strncpy(this->pname, in_pname.c_str(), 50);
//     }
// };

int m_shm_id = -1;  // 共享内存的引用 id
StProcInfo* m_shm = nullptr;    // 指向共享内存的地址空间
int m_pos = -1;     // 存放当前进程在共享内存中，数组下标的位置

void EXIT(int sig);


int main(int argc, char* argv[]) {

    // 处理程序的退出信号
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    // 创建/获取共享内存
    m_shm_id = shmget((key_t)0x5095, 1000 * sizeof(StProcInfo), 0666 | IPC_CREAT);
    if (m_shm_id == -1) {
        printf("创建 / 获取共享内存(%x)失败\n", 0x5095);
        return -1;
    }

    // 将共享内存连接到当前进程的地址空间，返回共享内存的首地址
    m_shm = (StProcInfo*)shmat(m_shm_id, NULL, 0);

    // 把当前进程的信息填充到结构体中
    StProcInfo proc_info(getpid(), "server1", 30, time(0));

    csemp sem_lock;     // 用于给共享内存加锁的信号量 id

    if (sem_lock.init(0x5095) == false) {   // 初始化信号量
        printf("创建 / 获取信号量(%x)失败。\n", 0x5095);
        EXIT(-1);
    }

    sem_lock.wait();    // 给申请共享内存部分的代码加锁

    /*
        如果有一个进程异常退出，没有清理自己的心跳信息，它的进程信息将残留在共享内存中，
        当前进程可能会重用，残留在共享内存中的进程 id，
        所以，如果共享内存中已存在当前进程编号，一定是其它进程残留的信息，当前进程应该重用这个位置。
    */
    for (int i = 0;i < 1000;++i) {
        if ((m_shm + i)->pid == proc_info.pid) {    // 创建共享内存时，内存中的数据默认初始化为0
            m_pos = i;
            printf("找到旧位置: i = %d\n", i);
            break;
        }
    }

    if (m_pos == -1) {
        // 在共享内存中寻找一个空的位置，把当前进程结构体保存到共享内存中
        for (int i = 0;i < 1000;++i) {
            if ((m_shm + i)->pid == 0) {    // 创建共享内存时，内存中的数据默认初始化为0
                m_pos = i;
                printf("找到新位置: i = %d\n", i);
                break;
            }
        }
    }

    // m_pos == -1 表示共享内存空间用完
    if (m_pos == -1) {
        sem_lock.post();    // 没找到位置，释放锁

        printf("共享内存空间已用完。\n");
        EXIT(-1);
    }

    // 把当前进程的结构体信息保存到共享内存中
    memcpy(m_shm + m_pos, &proc_info, sizeof(StProcInfo));

    sem_lock.post();    // 找到共享内存中的位置了，释放锁

    while (1) {
        printf("服务程序正在运行中...\n");

        // 更新进程的心跳信息
        sleep(25);
        (m_shm + m_pos)->atime = time(0);
    }

    return 0;
}

// 程序退出和信号 2、15 的处理函数，进程异常退出，不能执行此函数，会导致残留信息
void EXIT(int sig) {
    printf("sig = %d\n", sig);
    // 从共享内存中删除当前进程的心跳信息
    if (m_pos != -1) {
        memset(m_shm + m_pos, 0, sizeof(StProcInfo));   // 初始化内存空间为 0
    }

    // 把共享内存从当前进程分离
    if (m_shm_id != -1) {
        shmdt(m_shm);
    }

    exit(0);
}