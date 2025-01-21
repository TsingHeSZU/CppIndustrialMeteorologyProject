/*
    使用 tcp 协议，实现文件上传的客户端
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

StArg st_arg;               // 程序运行参数（xml）的结构体对象
clogfile logfile;           // 日志对象
ctcpclient tcp_client;      // 创建 tcp 通讯的客户端对象
string str_send_buffer;     // 发送报文的 buffer
string str_recv_buffer;     // 接收报文的 buffer

void _help();               // 程序帮助文档
bool parseXML(const char* str_xml_buffer);  // 把 xml 解析到 StArg 结构体变量中
void EXIT(int sig);         // 程序退出和信号 2、15 的处理函数
bool activeTest();          // tcp 长连接心跳机制
bool loginInfo(const char* argv);   // 向服务端发送登录报文，把客户端程序的参数传递给服务端

int main(int argc, char* argv[]) {
    if (argc != 3) {
        _help();
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

    // 解析 xml，得到程序的运行参数
    if (parseXML(argv[2]) == false) {
        logfile.write("parseXML(%s) failed.\n", argv[2]);
        return -1;
    }

    // 向服务端发起连接请求
    if (tcp_client.connect(st_arg.ip, st_arg.port) == false) {
        logfile.write("tcp_client.connect(%s, %d) failed.\n", st_arg.ip, st_arg.port);
        EXIT(-1);
    }

    // 向服务端发送登录报文，把客户端程序的参数传递给服务端
    if (loginInfo(argv[2]) == false) {
        logfile.write("loginInfo(%s) failed.\n", argv[2]);
        EXIT(-1);
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

// 程序帮助文档
void _help() {
    printf("\n");
    printf("Using: /CppIndustrialMeteorologyProject/tools/bin/tcpputfiles logfilename xmlbuffer\n\n");

    printf("Example: /CppIndustrialMeteorologyProject/tools/bin/procctl 20 "\
        "/CppIndustrialMeteorologyProject/tools/bin/tcpputfiles /log/idc/tcpputfiles_surfdata.log "\
        "\"<clienttype>1</clienttype>"
        "<ip>127.0.0.1</ip>"\
        "<port>5005</port>"\
        "<ptype>1</ptype>"\
        "<clientpath>/tmp/client</clientpath>"\
        "<clientpathbak>/tmp/client_tcp_bak</clientpathbak>"\
        "<ischild>true</ischild>"\
        "<matchname>*.xml,*.txt,*.csv</matchname>"\
        "<serverpath>/tmp/server</serverpath>"\
        "<timetvl>10</timetvl>"\
        "<timeout>50</timeout>"\
        "<pname>tcpputfiles_surfdata</pname>\"\n\n");

    printf("本程序是数据中心的公共功能模块, 采用tcp协议把文件上传给服务端。\n");
    printf("logfilename: 本程序运行的日志文件; \n");
    printf("xmlbuffer: 本程序运行的参数，如下: \n");
    printf("ip: 服务端的IP地址; \n");
    printf("port: 服务端的端口; \n");
    printf("ptype: 文件上传成功后的处理方式: 1-删除文件, 2-移动到备份目录;\n");
    printf("clientpath: 本地文件存放的根目录;\n");
    printf("clientpathbak: 文件成功上传后, 本地文件备份的根目录, 当 ptype==2 时有效;\n");
    printf("ischild: 是否上传 clientpath 目录下各级子目录的文件, true-是, false-否, 缺省为false;\n");
    printf("matchname: 待上传文件名的匹配规则，如\"*.TXT,*.XML\";\n");
    printf("serverpath: 服务端文件存放的根目录; \n");
    printf("timetvl: 扫描本地目录文件的时间间隔, 单位: 秒, 取值在 1-30 之间;\n");
    printf("timeout: 本程序的超时时间, 单位: 秒, 视文件大小和网络带宽而定, 建议设置50以上。\n");
}

// 把 xml 解析到 StArg 结构体变量中
bool parseXML(const char* str_xml_buffer) {
    memset(&st_arg, 0, sizeof(StArg));

    getxmlbuffer(str_xml_buffer, "clienttype", st_arg.client_type);
    if (st_arg.client_type == 0) {
        logfile.write("client_type is null.\n");
        return false;
    }

    getxmlbuffer(str_xml_buffer, "ip", st_arg.ip);
    if (strlen(st_arg.ip) == 0) {
        logfile.write("ip is null.\n");
        return false;
    }

    getxmlbuffer(str_xml_buffer, "port", st_arg.port);
    if (st_arg.port == 0) {
        logfile.write("port is null.\n");
        return false;
    }

    getxmlbuffer(str_xml_buffer, "ptype", st_arg.ptype);
    if ((st_arg.ptype != 1) && (st_arg.ptype != 2)) {
        logfile.write("ptype not in (1,2).\n");
        return false;
    }

    getxmlbuffer(str_xml_buffer, "clientpath", st_arg.client_path);
    if (strlen(st_arg.client_path) == 0) {
        logfile.write("clientpath is null.\n");
        return false;
    }

    getxmlbuffer(str_xml_buffer, "clientpathbak", st_arg.client_pathbak);
    if ((st_arg.ptype == 2) && (strlen(st_arg.client_pathbak) == 0)) {
        logfile.write("clientpathbak is null.\n");
        return false;
    }

    getxmlbuffer(str_xml_buffer, "ischild", st_arg.is_child);

    getxmlbuffer(str_xml_buffer, "serverpath", st_arg.server_path);
    if (strlen(st_arg.server_path) == 0) {
        logfile.write("srvpath is null.\n");
        return false;
    }

    getxmlbuffer(str_xml_buffer, "matchname", st_arg.matchname);
    if (strlen(st_arg.matchname) == 0) {
        logfile.write("matchname is null.\n");
        return false;
    }

    getxmlbuffer(str_xml_buffer, "timetvl", st_arg.timetvl);
    if (st_arg.timetvl == 0) {
        logfile.write("timetvl is null.\n");
        return false;
    }

    // 扫描本地目录文件的时间间隔（执行上传任务的时间间隔，程序休眠），单位：秒
    // st_arg.timetvl没有必要超过 30 秒
    if (st_arg.timetvl > 30) {
        st_arg.timetvl = 30;
    }

    // 进程心跳的超时时间，一定要大于 st_arg.timetvl
    getxmlbuffer(str_xml_buffer, "timeout", st_arg.timeout);
    if (st_arg.timeout == 0) {
        logfile.write("timeout is null.\n");
        return false;
    }
    if (st_arg.timeout <= st_arg.timetvl) {
        logfile.write("st_arg.timeout(%d) <= st_arg.timetvl(%d).\n", st_arg.timeout, st_arg.timetvl);
        return false;
    }

    // 进程名可以为空，主要是进程心跳结构体参数
    getxmlbuffer(str_xml_buffer, "pname", st_arg.pname, 50);
    // if (strlen(st_arg.pname) == 0) {
    //     logfile.write("pname is null.\n");
    //     return false;
    // }

    return true;
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

// 向服务端发送登录报文，把客户端程序的参数传递给服务端
bool loginInfo(const char* argv) {
    // 向服务端发送请求报文
    logfile.write("发送: %s\n", argv);
    if (tcp_client.write(argv) == false) {
        logfile.write("tcp_client.write(%s) failed.\n", argv);
        return false;
    }

    // 接收服务端的回应报文
    if (tcp_client.read(str_recv_buffer, 20) == false) {
        logfile.write("tcp_client.read() failed.\n");
        return false;
    }
    logfile.write("接收: %s\n", str_recv_buffer.c_str());

    logfile.write("login (%s, %d) success.\n", st_arg.ip, st_arg.port);
    return true;
}