/*
    使用 tcp 协议，实现文件传输的服务端
*/
#include "/CppIndustrialMeteorologyProject/public/_public.h"
using namespace idc;

// 程序运行参数的结构体（xml部分）
typedef struct StArg {
    int client_type;            // 客户端类型，1-上传文件，2-下载文件，本程序固定填1
    char ip[31];                // 服务端的IP地址
    int port;                   // 服务端的端口
    char client_path[256];      // 本地文件存放的根目录: /data /data/aaa /data/bbb
    int ptype;                  // 文件上传成功后本地文件的处理方式：1-删除文件，2-移动到备份目录
    char client_pathbak[256];   // 文件成功上传后，本地文件备份的根目录，当 ptype==2 时有效。
    bool is_child;              // 是否上传 client_path 目录下各级子目录的文件，true-是，false-否
    char matchname[256];        // 待上传文件名的匹配规则，如"*.TXT,*.XML"
    char server_path[256];      // 服务端文件存放的根目录。/data1 /data1/aaa /data1/bbb
    int timetvl;                // 扫描本地目录文件的时间间隔（执行文件上传任务的时间间隔，程序休眠），单位：秒
    int timeout;                // 进程心跳的超时时间
    char pname[51];             // 进程名，建议用"tcpputfiles_后缀"的方式 
}StArg;

clogfile logfile;           // 服务程序的运行日志
ctcpserver tcp_server;      // 创建 tcp 通信的服务端对象
string str_send_buffer;     // 发送报文的 buffer
string str_recv_buffer;     // 接收报文的 buffer
StArg st_arg;               // 存储客户端程序运行参数的结构体对象

void FatherEXIT(int sig);   // 父进程退出函数
void ChildEXIT(int sig);    // 子进程退出函数
void recvFilesMain();       // 上传文件的主函数
bool clientLogin();         // 处理登录客户端的登录报文       

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Using: ./tcpfileserver port logfile\n");
        printf("Example: ./tcpfileserver 5005 /log/idc/tcpfileserver.log\n");
        printf("    /CppIndustrialMeteorologyProject/tools/bin/procctl 10 "\
            "/CppIndustrialMeteorologyProject/tools/bin/tcpfileserver 5005 /log/idc/tcpfileserver.log\n\n");
        return -1;
    }

    /*
        关闭全部的信号和输入输出
        设置信号，在 shell 状态下可用 "kill 进程号" 正常终止进程
        但请不要用 "kill -9 进程号" 强行终止（导致资源无法正常回收）
        在网络通讯程序中，一般不关IO，因为某些函数可能会往 1 和 2 中输出信息
        如果关了 1 和 2，那么 1 和 2 会被 socket 重用，向 1 和 2 输出的信息会发送到网络中
    */
    //closeioandsignal(false);
    signal(SIGINT, FatherEXIT);
    signal(SIGTERM, FatherEXIT);

    if (logfile.open(argv[2]) == false) {
        printf("logfile.open(%s) failed.\n", argv[2]);
        return -1;
    }

    // 初始化监听用的端口号
    if (tcp_server.initserver(atoi(argv[1])) == false) {
        logfile.write("tcp_server.initserver(%s) failed.\n", argv[1]);
        return -1;
    }

    while (true) {
        // 等待客户端的连接请求
        if (tcp_server.accept() == false) {
            logfile.write("tcp_server.accept() failed.\n");
            FatherEXIT(-1);
        }

        logfile.write("客户端 (%s) 已连接\n", tcp_server.getip());

        if (fork() > 0) {
            // 父进程关闭通信用的端口号（accept函数会返回通信用的 fd）
            tcp_server.closeclient();
            continue;
        }

        // 子进程重新设置退出信号
        signal(SIGINT, ChildEXIT);
        signal(SIGTERM, ChildEXIT);

        // 子进程关闭监听用的端口号（继承了父进程监听的 fd）
        tcp_server.closelisten();

        // 子进程与客户端进行通信，处理业务

        // 处理客户端的登录报文
        if (clientLogin() == false) {
            ChildEXIT(-1);
        }

        if (st_arg.client_type == 1) {
            // 如果 st_arg.client_type == 1, 调用上传文件的主函数
            recvFilesMain();
        }
        else if (st_arg.client_type == 2) {
            // 如果 st_arg.client_type == 2, 调用下载文件的主函数
            //sendFilesMain();
        }

        ChildEXIT(0);
    }
    return 0;
}

// 父进程退出函数
void FatherEXIT(int sig) {
    // 以下代码是为了防止信号处理函数在执行的过程中被信号中断
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    logfile.write("父进程退出, sig = %d;\n", sig);
    tcp_server.closelisten();   // 关闭监听用的 fd(socket)

    kill(0, 15);        // 通知全部的子进程退出
    exit(0);
}

// 子进程退出函数
void ChildEXIT(int sig) {
    // 以下代码是为了防止信号处理函数在执行的过程中被信号中断
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    logfile.write("子进程退出, sig = %d;\n", sig);
    tcp_server.closeclient();   // 关闭通信用的 fd(socket)

    exit(0);
}

// 上传文件的主函数
void recvFilesMain() {
    while (true) {
        // 接收客户端的报文
        if (tcp_server.read(str_recv_buffer, 60) == false) {
            logfile.write("tcp_server.read() failed.\n");
            return;
        }
        logfile.write("str_recv_buffer = %s\n", str_recv_buffer.c_str());

        // 处理 tcp 长连接心跳机制的报文
        if (str_recv_buffer == "<activetest>ok</activetest>") {
            str_send_buffer = "ok";
            logfile.write("str_send_buffer = %s\n", str_send_buffer.c_str());
            if (tcp_server.write(str_send_buffer) == false) {
                logfile.write("tcp_server.write() failed.\n");
                return;
            }
        }
    }
}

// 处理登录客户端的登录报文
bool clientLogin() {
    // 接收客户端的登录报文
    if (tcp_server.read(str_recv_buffer, 20) == false) {
        logfile.write("tcp_server.read() failed.\n");
        return false;
    }
    logfile.write("str_recv_buffer = %s\n", str_recv_buffer.c_str());

    // 解析客户端的登录报文，不需要对参数做合法性判断，客户端已经判断过了
    memset(&st_arg, 0, sizeof(StArg));
    getxmlbuffer(str_recv_buffer, "clienttype", st_arg.client_type);
    getxmlbuffer(str_recv_buffer, "clientpath", st_arg.client_path);
    getxmlbuffer(str_recv_buffer, "serverpath", st_arg.server_path);

    // 为什么要判断客户端的类型？不是只有 1 和 2 吗？防止非法的连接请求
    if (st_arg.client_type != 1 && st_arg.client_type != 2) {
        str_send_buffer = "failed";
    }
    else {
        str_send_buffer = "ok";
    }

    if (tcp_server.write(str_send_buffer) == false) {
        logfile.write("tcp_server.write(%s) failed.\n", str_send_buffer.c_str());
        return false;
    }

    logfile.write("%s login %s.\n%s\n", tcp_server.getip(), str_send_buffer.c_str(), str_recv_buffer.c_str());
    return true;
}
