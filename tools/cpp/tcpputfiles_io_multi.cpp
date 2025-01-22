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
    char pname[51];             // 进程名，建议用"tcpputfiles_io_multi_后缀"的方式 
}StArg;

StArg st_arg;               // 程序运行参数（xml）的结构体对象
clogfile logfile;           // 日志对象
ctcpclient tcp_client;      // 创建 tcp 通讯的客户端对象
string str_send_buffer;     // 发送报文的 buffer
string str_recv_buffer;     // 接收报文的 buffer
Cpactive pactive;           // 进程心跳

void _help();               // 程序帮助文档
bool parseXML(const char* str_xml_buffer);  // 把 xml 解析到 StArg 结构体变量中
void EXIT(int sig);         // 程序退出和信号 2、15 的处理函数
bool activeTest();          // tcp 长连接心跳机制
bool loginInfo(const char* argv);   // 向服务端发送登录报文，把客户端程序的参数传递给服务端
bool tcpPutFiles(bool& bcontinue);         // 文件上传的主函数，执行一次文件上传的任务
bool ackMessage(const string& tmp_str_recv_buffer);     // 处理传输文件的响应报文（删除或者转存本地的文件）
bool sendFile(const string& filename, const int filesize);   // 把文件的内容发送给对端

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

    // 如果调用了 tcpPutFiles() 发送了文件，bcontinue 为 true，否则为 false，初始化为 true
    bool bcontinue = true;

    while (true) {
        // 调用文件上传的主函数，执行一次文件上传的任务
        if (tcpPutFiles(bcontinue) == false) {
            logfile.write("tcpPutFiles() failed.\n");
            EXIT(-1);
        }

        // 如果刚才执行文件上传任务的时候，上传了文件，那么，在上传的过程中，可能有新的文件陆续生成
        // 为了保证文件被尽快上传，进程不休眠（没有执行文件上传任务的时候，进程才休眠）
        if (bcontinue == false) {
            // 当客户端没有上传文件时，为了保证 TCP 长连接不断开（当服务端 read 一直没有读到数据时，会主动断开），需要发送心跳报文
            sleep(st_arg.timetvl);
            // 发送心跳报文
            if (activeTest() == false) {
                break;
            }
        }

        // 执行一次文件上传任务，更新进程的心跳信息 
        pactive.updateAtime();
    }

    EXIT(0);
}

// 程序帮助文档
void _help() {
    printf("\n");
    printf("Using: /CppIndustrialMeteorologyProject/tools/bin/tcpputfiles_io_multi logfilename xmlbuffer\n\n");

    printf("Example: /CppIndustrialMeteorologyProject/tools/bin/procctl 20 "\
        "/CppIndustrialMeteorologyProject/tools/bin/tcpputfiles_io_multi /log/idc/tcpputfiles_io_multi_surfdata.log "\
        "\"<clienttype>1</clienttype>"\
        "<ip>127.0.0.1</ip>"\
        "<port>5005</port>"\
        "<ptype>1</ptype>"\
        "<clientpath>/tmp/client</clientpath>"\
        "<clientpathbak>/tmp/client_tcp_bak</clientpathbak>"\
        "<ischild>true</ischild>"\
        "<matchname>*.xml,*.txt,*.csv,*.json</matchname>"\
        "<serverpath>/tmp/server</serverpath>"\
        "<timetvl>10</timetvl>"\
        "<timeout>65</timeout>"\
        "<pname>tcpputfiles_io_multi_surfdata</pname>\"\n\n");

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
    // logfile.write("发送: %s\n", str_send_buffer.c_str());

    // 接收服务端的回应报文
    if (tcp_client.read(str_recv_buffer, 10) == false) {
        printf("tcp_client.read(%s, 10) failed.\n", str_recv_buffer.c_str());
        return false;
    }
    // logfile.write("接收: %s\n", str_recv_buffer.c_str());
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

// 文件上传的主函数，执行一次文件上传的任务
bool tcpPutFiles(bool& bcontinue) {
    // 标记是否有上传文件的任务
    bcontinue = false;

    cdir dir;

    // 打开 st_arg.client_path 目录
    if (dir.opendir(st_arg.client_path, st_arg.matchname, 10000, st_arg.is_child) == false) {
        logfile.write("dir.opendir(%s) failed.\n", st_arg.client_path);
        return false;
    }

    // 未收到服务端确认报文的文件数量，发送了一个文件就加 1，接收了一个回应就减 1
    int delayed = 0;

    // 遍历目录中的每个文件
    while (dir.readdir()) {
        bcontinue = true;
        // 把文件名、修改时间、文件大小组成报文段，发送给服务端
        sformat(str_send_buffer, "<filename>%s</filename><mtime>%s</mtime><size>%d</size>",
            dir.m_ffilename.c_str(), dir.m_mtime.c_str(), dir.m_filesize);
        // logfile.write("str_send_buffer = %s\n", str_send_buffer.c_str());
        if (tcp_client.write(str_send_buffer) == false) {
            logfile.write("tcp_client.write() failed.\n");
            return false;
        }

        // 发送文件内容
        logfile.write("send %s(%d)...", dir.m_ffilename.c_str(), dir.m_filesize);
        if (sendFile(dir.m_ffilename, dir.m_filesize) == false) {
            logfile << "failed.\n";
            tcp_client.close();
            return false;
        }
        else {
            logfile << "ok.\n";
            ++delayed;
        }

        // 上传一个文件，更新进程心跳
        pactive.updateAtime();

        while (delayed > 0) {
            // 接收服务端的确认报文
            if (tcp_client.read(str_recv_buffer, -1) == false) {
                break;
            }
            // logfile.write("str_recv_buffer = %s\n", str_recv_buffer.c_str());

            // 处理服务端的确认报文（删除本地文件或把本地文件移动到备份目录）
            ackMessage(str_recv_buffer);
            --delayed;
        }
    }

    // 继续接收服务端的确认报文（防止确认报文没有接收完）
    while (delayed > 0) {
        // 接收服务端的确认报文
        if (tcp_client.read(str_recv_buffer, 10) == false) {
            break;
        }
        // logfile.write("str_recv_buffer = %s\n", str_recv_buffer.c_str());

        // 处理服务端的确认报文（删除本地文件或把本地文件移动到备份目录）
        ackMessage(str_recv_buffer);
        --delayed;
    }

    return true;
}

// 处理传输文件的响应报文（删除或者转存本地的文件）
bool ackMessage(const string& tmp_str_recv_buffer) {
    // 服务端给的确认报文格式：<filename>%s</filename><result>ok</result>
    string filename;    // 本地文件名
    string result;      // 对端接收文件的结果
    getxmlbuffer(tmp_str_recv_buffer, "filename", filename);
    getxmlbuffer(tmp_str_recv_buffer, "result", result);

    // 如果服务端接收文件不成功，直接返回（下次执行文件传输任务时将会重传）
    if (result != "ok") {
        return true;
    }

    // 如果 st_arg.ptype == 1, 删除文件
    if (st_arg.ptype == 1) {
        if (remove(filename.c_str()) != 0) {
            logfile.write("remove(%s) failed.\n", filename.c_str());
            return false;
        }
    }

    // 如果 st_arg.ptype == 2, 移动到备份目录
    if (st_arg.ptype == 2) {
        // 生成转存后的备份目录文件名，例如：/tmp/client/2.xml /tmp/clientbak/2.xml
        string bak_filename = filename;
        replacestr(bak_filename, st_arg.client_path, st_arg.client_pathbak, false);     // 最后一个参数为 false
        if (renamefile(filename, bak_filename) == false) {
            logfile.write("renamefile(%s,%s) failed.\n", filename.c_str(), bak_filename.c_str());
            return false;
        }
    }
    return true;
}

// 把文件的内容发送给对端
bool sendFile(const string& filename, const int filesize) {
    int on_read = 0;        // 每次打算从文件中读取的字节数
    char buffer[1000];      // 存放读取数据的 buffer，buffer 的大小可以参考硬盘一次读取数据量（4k 为宜）
    int total_bytes = 0;    // 从文件中已读取的字节总数
    cifile ifile;           // 读取文件对象

    // 必须以二进制的方式操作文件
    if (ifile.open(filename, ios::binary | ios::in) == false) {
        logfile.write("ifile.open(%s, ...) failed.\n", filename.c_str());
        return false;
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        if (filesize - total_bytes > 1000) {
            on_read = 1000;
        }
        else {
            on_read = filesize - total_bytes;
        }

        // 从文件中读数据
        ifile.read(buffer, on_read);

        // 把读取到的数据发送给对端
        if (tcp_client.write(buffer, on_read) == false) {
            logfile.write("tcp_client.write() failed.\n");
            return false;
        }

        // 计算文件已读取的字节总数，如果文件已读完，跳出循环
        total_bytes = total_bytes + on_read;
        if (total_bytes == filesize) {
            break;
        }
    }
    return true;
}
