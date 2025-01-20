// 模拟网上银行 app 服务端，增加心跳报文
#include"/CppIndustrialMeteorologyProject/public/_public.h"
using namespace idc;

ctcpserver tcp_server;      // TCP 服务器端
clogfile logfile;       // 服务程序运行日志

void FatherEXIT(int sig);     // 父进程退出函数
void ChildEXIT(int sig);     // 子进程退出函数

string str_send_buffer;     // 发送报文的 buffer
string str_recv_buffer;     // 接收报文的 buffer

bool bizmain();     // 业务处理主函数

int main(int argc, char* argv[]) {

    if (argc != 4) {
        printf("Using: ./demo06 port logfile timeout\n");
        printf("Example: ./demo06 5005 /log/idc/demo06.log 10\n\n");
        return -1;
    }

    /*
        关闭全部的信号和输入输出
        设置信号, 在shell状态下可用 "kill 进程号" 正常终止进程
        但请不要用 "kill -9 进程号" 强行终止
    */
    signal(SIGINT, FatherEXIT);
    signal(SIGTERM, FatherEXIT);

    if (logfile.open(argv[2])==false) { 
        printf("logfile.open(%s) failed.\n",argv[2]); 
        return -1; 
    }

    // 服务端初始化，创建监听用的端口号
    if (tcp_server.initserver(atoi(argv[1]))==false){
        logfile.write("tcp_server.initserver(%s) failed.\n",argv[1]); 
        return -1;
    }

    while(true){
        // 接受客户端的连接请求，accept() 会阻塞，直到有客户端发送连接请求
        if(tcp_server.accept() == false){
            logfile.write("tcp_server.accept() failed.\n"); 
            FatherEXIT(-1);
        }

        logfile.write("客户端 (%s) 已连接\n",tcp_server.getip());

        if (fork() > 0) {
            // 父进程继续执行 accept()
            tcp_server.closeclient();
            continue; 
        } 

        // 子进程设置退出信号
        signal(SIGINT,ChildEXIT); 
        signal(SIGTERM,ChildEXIT);

        tcp_server.closelisten();   // 子进程关闭监听用的 端口号

        while(true){
            // 子进程与客户端进行通讯，处理业务
            if (tcp_server.read(str_recv_buffer, atoi(argv[3]))==false){
                logfile.write("tcp_server.read() failed.\n"); 
                ChildEXIT(0);
            }
            logfile.write("接收: %s\n", str_recv_buffer.c_str());   

            bizmain();      // 业务处理子函数

            if (tcp_server.write(str_send_buffer)==false){
                logfile.write("tcp_server.send() failed.\n"); 
                ChildEXIT(0);
            }
            logfile.write("发送: %s\n", str_send_buffer.c_str());                     
        }
        ChildEXIT(0);
    }
    return 0;
}

void FatherEXIT(int sig) {
    // 以下代码是为了防止信号处理函数在执行的过程中被信号中断
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    logfile.write("父进程退出, sig = %d;\n", sig);
    tcp_server.closelisten();   // 关闭监听的 socket

    kill(0, 15);    // 通知全部的子进程退出
    exit(0);
}

void ChildEXIT(int sig) {
    // 以下代码是为了防止信号处理函数在执行的过程中被信号中断
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    logfile.write("子进程退出, sig = %d;\n", sig);

    tcp_server.closeclient();   // 关闭客户端的 socket
    exit(0);
}

void biz001();      // 登录
void biz002();      // 查询余额
void biz003();      // 转账

// 业务处理子函数
bool bizmain() {
    int bizid;  // 业务代码

    getxmlbuffer(str_recv_buffer, "bizid", bizid);

    switch (bizid){
        case 0:     // 心跳
            str_send_buffer = "<retcode>0</retcode>";
            break; 
        case 1:    // 登录
            biz001();
            break;
        case 2:    // 查询余额
            biz002();
            break;
        case 3:    // 转帐
            biz003();
            break;
        default:   // 非法报文
            str_send_buffer = "<retcode>9</retcode><message>业务不存在</message>";
            break;
    }

    return true;
}

// 登录
void biz001(){
    string username,password;
    getxmlbuffer(str_recv_buffer,"username",username);
    getxmlbuffer(str_recv_buffer,"password",password);

    if (username=="13922200000" && password=="123456"){
        str_send_buffer="<retcode>0</retcode><message>成功</message>";
    }
    else{
        str_send_buffer="<retcode>-1</retcode><message>用户名或密码不正确</message>";
    }
}   

// 查询余额
void biz002(){
    string cardid;
    getxmlbuffer(str_recv_buffer,"cardid",cardid);  // 获取卡号

    // 假装操作了数据库，得到了卡的余额

    str_send_buffer="<retcode>0</retcode><ye>128.83</ye>";
}   

// 转账
void biz003(){
    string cardid1,cardid2;
    double je;

    getxmlbuffer(str_recv_buffer,"cardid1",cardid1);
    getxmlbuffer(str_recv_buffer,"cardid2",cardid2);
    getxmlbuffer(str_recv_buffer,"je",je);

    // 假装操作了数据库，更新了两个账户的金额，完成了转帐操作。

    if (je < 100){
        str_send_buffer="<retcode>0</retcode><message>成功</message>";
    }
    else{
        str_send_buffer="<retcode>-1</retcode><message>余额不足</message>";
    }
}      