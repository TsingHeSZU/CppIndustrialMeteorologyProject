/*
    使用 tcp 协议，实现文件传输的服务端
*/
#include "/CppIndustrialMeteorologyProject/public/_public.h"
using namespace idc;

clogfile logfile;           // 服务程序的运行日志
ctcpserver tcp_server;      // 创建 tcp 通信的服务端对象
string str_send_buffer;     // 发送报文的 buffer
string str_recv_buffer;     // 接收报文的 buffer

void FatherEXIT(int sig);   // 父进程退出函数
void ChildEXIT(int sig);    // 子进程退出函数
void recvFilesMain();       // 上传文件的主函数       

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

        recvFilesMain();    // 上传文件的主函数

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
