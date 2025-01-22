/*
    使用 tcp 协议，实现下载文件的客户端
*/
#include "/CppIndustrialMeteorologyProject/public/_public.h"
using namespace idc;

// 程序运行参数的结构体（xml部分）
typedef struct StArg {
    int client_type;            // 客户端类型，1-上传文件，2-下载文件，本程序固定填2
    char ip[31];                // 服务端的IP地址
    int port;                   // 服务端的端口
    char server_path[256];      // 服务端文件存放的根目录: /data1 /data1/aaa /data1/bbb
    int ptype;                  // 文件下载成功后服务端文件的处理方式：1-删除文件，2-移动到备份目录
    char server_pathbak[256];   // 文件成功下载后，服务端文件备份的根目录，当 ptype==2 时有效
    bool is_child;              // 是否下载站 server_path 目录下各级子目录的文件，true-是，false-否
    char matchname[256];        // 待下载文件名的匹配规则，如"*.TXT,*.XML"
    char client_path[256];      // 客户端文件存放的根目录: /data /data/aaa /data/bbb
    int timetvl;                // 不下载文件时，客户端程序发送心跳报文的时间间隔，单位：秒
    int timeout;                // 进程心跳的超时时间
    char pname[51];             // 进程名，建议用"tcpgetfiles_后缀"的方式 
}StArg;


clogfile logfile;           // 日志对象
StArg st_arg;               // 程序运行参数（xml）的结构体对象
ctcpclient tcp_client;      // 创建 tcp 通讯的客户端对象
string str_send_buffer;     // 发送报文的 buffer
string str_recv_buffer;     // 接收报文的 buffer
Cpactive pactive;           // 进程心跳

void EXIT(int sig);         // 程序退出和信号 2、15 的处理函数
bool activeTest();          // tcp 长连接心跳机制
void _help();               // 程序帮助文档
bool parseXML(const char* str_xml_buffer);  // 把 xml 解析到 StArg 结构体变量中
bool loginInfo(const char* argv);           // 向服务端发送登录报文，把客户端程序的参数传递给服务端
void tcpGetFiles();         // 文件下载的主函数
bool recvFilesContent(const string& filename, const string& m_time, int file_size);    // 处理客户端上传文件的内容


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

    // 把进程心跳信息写入共享内存
    pactive.addProcInfo(st_arg.timeout, st_arg.pname);

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

    // 调用文件下载的主函数
    tcpGetFiles();

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
    // logfile.write("发送: %s\n", str_send_buffer.c_str());

    // 接收服务端的回应报文
    if (tcp_client.read(str_recv_buffer, 10) == false) {
        printf("tcp_client.read(%s, 10) failed.\n", str_recv_buffer.c_str());
        return false;
    }
    // logfile.write("接收: %s\n", str_recv_buffer.c_str());
    return true;
}

// 程序帮助文档
void _help() {
    printf("\n");
    printf("Using: /CppIndustrialMeteorologyProject/tools/bin/tcpgetfiles logfilename xmlbuffer\n\n");

    printf("Example: /CppIndustrialMeteorologyProject/tools/bin/procctl 20 "\
        "/CppIndustrialMeteorologyProject/tools/bin/tcpgetfiles /log/idc/tcpgetfiles_surfdata.log "\
        "\"<clienttype>2</clienttype>"\
        "<ip>127.0.0.1</ip>"\
        "<port>5005</port>"\
        "<ptype>1</ptype>"\
        "<serverpath>/tmp/server</serverpath>"\
        "<serverpathbak>/tmp/server_tcp_bak</serverpathbak>"\
        "<ischild>true</ischild>"\
        "<clientpath>/tmp/client</clientpath>"\
        "<matchname>*.xml,*.txt,*.csv,*.json</matchname>"\
        "<timetvl>10</timetvl>"\
        "<timeout>50</timeout>"\
        "<pname>tcpgetfiles_surfdata</pname>\"\n\n");

    printf("本程序是数据中心的公共功能模块, 采用 TCP 协议从服务端下载文件。\n");
    printf("logfilename: 本程序运行的日志文件; \n");
    printf("xmlbuffer: 本程序运行的参数，如下: \n");
    printf("ip: 服务端的IP地址; \n");
    printf("port: 服务端的端口; \n");
    printf("ptype: 文件下载成功后的处理方式: 1-删除文件, 2-移动到备份目录;\n");
    printf("serverpath: 服务端文件存放的根目录; \n");
    printf("serverpathbak: 文件成功下载后, 服务端文件备份的根目录, 当 ptype==2 时有效;\n");
    printf("ischild: 是否下载 serverpath 目录下各级子目录的文件, true-是, false-否, 缺省为false;\n");
    printf("clientpath: 本地文件存放的根目录;\n");
    printf("matchname: 待上传文件名的匹配规则，如\"*.TXT,*.XML\";\n");
    printf("timetvl: 扫描本地目录文件的时间间隔, 单位: 秒, 取值在 1-30 之间;\n");
    printf("timeout: 本程序的超时时间, 单位: 秒, 视文件大小和网络带宽而定, 建议设置50以上;\n");
    printf("pname: 本程序的进程名。\n");
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

    getxmlbuffer(str_xml_buffer, "serverpath", st_arg.server_path);
    if (strlen(st_arg.server_path) == 0) {
        logfile.write("srvpath is null.\n");
        return false;
    }

    getxmlbuffer(str_xml_buffer, "serverpathbak", st_arg.server_pathbak);
    if ((st_arg.ptype == 2) && (strlen(st_arg.server_pathbak) == 0)) {
        logfile.write("serverpathbak is null.\n");
        return false;
    }

    getxmlbuffer(str_xml_buffer, "ischild", st_arg.is_child);

    getxmlbuffer(str_xml_buffer, "matchname", st_arg.matchname);
    if (strlen(st_arg.matchname) == 0) {
        logfile.write("matchname is null.\n");
        return false;
    }

    getxmlbuffer(str_xml_buffer, "clientpath", st_arg.client_path);
    if (strlen(st_arg.client_path) == 0) {
        logfile.write("clientpath is null.\n");
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

// 向服务端发送登录报文，把客户端程序的参数传递给服务端
bool loginInfo(const char* argv) {
    // 向服务端发送请求报文
    // logfile.write("发送: %s\n", argv);
    if (tcp_client.write(argv) == false) {
        logfile.write("tcp_client.write(%s) failed.\n", argv);
        return false;
    }

    // 接收服务端的回应报文
    if (tcp_client.read(str_recv_buffer, 10) == false) {
        logfile.write("tcp_client.read() failed.\n");
        return false;
    }
    // logfile.write("接收: %s\n", str_recv_buffer.c_str());

    logfile.write("login (%s, %d) success.\n", st_arg.ip, st_arg.port);
    return true;
}

// 文件下载的主函数（处理服务端上传文件）
void tcpGetFiles() {
    while (true) {
        // 更新进程心跳
        pactive.updateAtime();

        // 接收服务端的报文
        if (tcp_client.read(str_recv_buffer, st_arg.timetvl + 10) == false) {
            logfile.write("tcp_client.read() failed.\n");
            return;
        }
        // logfile.write("str_recv_buffer = %s\n", str_recv_buffer.c_str());

        // 处理 tcp 长连接心跳机制的报文
        if (str_recv_buffer == "<activetest>ok</activetest>") {
            str_send_buffer = "ok";
            // logfile.write("str_send_buffer = %s\n", str_send_buffer.c_str());
            if (tcp_client.write(str_send_buffer) == false) {
                logfile.write("tcp_client.write() failed.\n");
                return;
            }
        }

        // 处理下载文件的请求报文
        if (str_recv_buffer.find("<filename>") != string::npos) {
            // 解析下载文件请求报文的 xml
            string server_filename;     // 服务端待下载的文件名
            string m_time;              // 文件的时间
            int file_size = 0;          // 文件大小
            getxmlbuffer(str_recv_buffer, "filename", server_filename);
            getxmlbuffer(str_recv_buffer, "mtime", m_time);
            getxmlbuffer(str_recv_buffer, "size", file_size);

            // 客户端和服务端文件的目录是不一样的，以下代码生成客户端的文件名
            // 把文件名中的 server_path 替换成 client_path，第三个参数填 false
            string client_filename;     // 服务端的文件名
            client_filename = server_filename;
            replacestr(client_filename, st_arg.server_path, st_arg.client_path, false);

            // 接收文件的内容
            logfile.write("recv %s(%d)...", client_filename.c_str(), filesize);
            if (recvFilesContent(client_filename, m_time, file_size) == true) {
                logfile << "ok.\n";
                sformat(str_send_buffer, "<filename>%s</filename><result>ok</result>", server_filename.c_str());
            }
            else {
                logfile << "failed.\n";
                sformat(str_send_buffer, "<filename>%s</filename><result>failed</result>", server_filename.c_str());
            }

            // 把确认报文发送给服务端
            // logfile.write("str_send_buffer = %s\n", str_send_buffer.c_str());
            if (tcp_client.write(str_send_buffer) == false) {
                logfile.write("tcp_client.write() failed.\n");
                return;
            }
        }
    }
}

// 接收服务端上传文件的内容（客户端下载）
bool recvFilesContent(const string& filename, const string& m_time, int filesize) {
    int total_bytes = 0;    // 已接收文件的总字节数
    int on_read = 0;        // 本次打算接收的字节数
    char buffer[1000];      // 接收文件内容的缓冲区
    cofile ofile;           // 写入文件的对象

    // 必须以二进制的方式操作文件
    if (ofile.open(filename, true, ios::out | ios::binary) == false) {
        logfile.write("ofile.open(%s, ...) failed.\n", filename.c_str());
        return false;
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        // 计算本次应该接收的字节数
        if (filesize - total_bytes > 1000) {
            on_read = 1000;
        }
        else {
            on_read = filesize - total_bytes;
        }

        // 接收文件内容
        if (tcp_client.read(buffer, on_read) == false) {
            logfile.write("tcp_client.read() failed.\n");
            return false;
        }

        // 把接收到的内容写入文件
        ofile.write(buffer, on_read);

        // 计算已接收文件的总字节数，如果文件接收完，跳出循环
        total_bytes = total_bytes + on_read;

        if (total_bytes == filesize) {
            break;
        }
    }

    // 关闭文件，把临时文件改为正式文件
    ofile.closeandrename();

    // 文件的修改时间与服务端一致
    setmtime(filename, m_time);

    return true;
}