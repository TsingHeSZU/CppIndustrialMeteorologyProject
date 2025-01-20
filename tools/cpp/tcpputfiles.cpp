/*
    使用 tcp 协议，实现文件上传的客户端
*/
#include "/CppIndustrialMeteorologyProject/public/_public.h"
using namespace idc;

clogfile logfile;           // 日志对象
ctcpclient tcp_client;      // 创建 tcp 通讯的客户端对象
string str_send_buffer;     // 发送报文的 buffer
string str_recv_buffer;     // 接收报文的 buffer

void EXIT(int sig);         // 程序退出和信号 2、15 的处理函数
bool activeTest();          // tcp 长连接心跳机制

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Using: ./tcpputfiles logfile ip port\n");
        printf("Example: ./tcpputfiles /log/idc/tcpputfiles.log 127.0.0.1 5005\n\n");
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
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    // 打开日志文件
    if (logfile.open(argv[1]) == false) {
        printf("打开日志文件失败 (%s) \n", argv[1]);
        return -1;
    }

    // 向服务端发起连接请求
    if (tcp_client.connect(argv[2], atoi(argv[3])) == false) {
        logfile.write("tcp_client.connect(%s, %d) failed.\n", argv[2], atoi(argv[3]));
        return -1;
    }

    while (true) {
        sleep(10);

        // 发送心跳报文
        if (activeTest() == false) {
            break;
        }
    }
    EXIT(0);
}

// 程序退出和信号 2、15 的处理函数
void EXIT(int sig) {
    logfile.write("程序退出, sig = %d\n\n", sig);
    exit(0);
}

// tcp 长连接心跳机制
bool activeTest() {
    str_send_buffer = "<activetest>ok</activetest>";

    // 向服务端发送请求报文
    if (tcp_client.write(str_send_buffer) == false) {
        printf("tcp_client.write(%s) failed.\n", str_send_buffer.c_str());
        return false;
    }
    logfile.write("发送: %s\n", str_send_buffer.c_str());

    // 接收服务端的回应报文
    if (tcp_client.read(str_recv_buffer, 60) == false) {
        printf("tcp_client.read(%s, 60) failed.\n", str_recv_buffer.c_str());
        return false;
    }
    logfile.write("接收: %s\n", str_recv_buffer.c_str());
    return true;
}