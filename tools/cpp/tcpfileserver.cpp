/*
    使用 tcp 协议，实现文件传输的服务端
*/
#include "_public.h"
using namespace idc;

// 程序运行参数的结构体（xml部分）
typedef struct StArg {
    int client_type;            // 客户端类型，1-上传文件，2-下载文件
    char client_path[256];      // 本地文件存放的根目录: /data /data/aaa /data/bbb
    int ptype;                  // 文件上传成功后本地文件的处理方式：1-删除文件，2-移动到备份目录
    char server_pathbak[256];   // 文件成功下载后，服务端文件备份的根目录，当 ptype==2 时有效
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
Cpactive pactive;           // 进程心跳

void FatherEXIT(int sig);   // 父进程退出函数
void ChildEXIT(int sig);    // 子进程退出函数
bool activeTest();          // TCP 长连接心跳机制
bool clientLogin();         // 处理客户端的登录报文，登录的作用是得到客户端程序的运行参数   
void recvFilesMain();       // 处理客户端上传文件的主函数
bool recvFilesContent(const string& filename, const string& m_time, int file_size);    // 处理客户端上传文件的内容
void sendFilesMain();       // 处理客户端下载文件的主函数
bool tcpPutFiles(bool& bcontinue);  // 文件下载的主函数，执行一次文件下载的任务
bool sendFile(const string& filename, const int filesize);   // 把文件的内容发送给客户端（当客户端需要下载文件）
bool ackMessage(const string& tmp_str_recv_buffer);     // 处理传输文件的响应报文（删除或者转存本地的文件）


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Using: ./tcpfileserver port logfile\n");
        printf("Example: /CppIndustrialMeteorologyProject/tools/bin/procctl 10 "\
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
        // 等待客户端的连接请求，accept() 函数会一直阻塞，所以不用将父进程信息加入进程心跳的共享内存中
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
            logfile.write("clientLogin(%s) failed.\n", tcp_server.getip());
            ChildEXIT(-1);
        }

        // 把进程的心跳信息写入共享内存（与客户端程序心跳信息保持一致，所以是在 clientLogin() 之后执行）
        // 每个子进程有自己的心跳
        pactive.addProcInfo(st_arg.timeout, st_arg.pname);

        if (st_arg.client_type == 1) {
            // 如果 st_arg.client_type == 1, 调用上传文件的主函数
            recvFilesMain();
        }
        else if (st_arg.client_type == 2) {
            // 如果 st_arg.client_type == 2, 调用下载文件的主函数
            sendFilesMain();
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

// TCP 长连接心跳机制
bool activeTest() {
    str_send_buffer = "<activetest>ok</activetest>";

    // 向客户端发送请求报文
    if (tcp_server.write(str_send_buffer) == false) {
        printf("tcp_server.write(%s) failed.\n", str_send_buffer.c_str());
        return false;
    }
    // logfile.write("发送: %s\n", str_send_buffer.c_str());

    // 接收客户端的回应报文
    if (tcp_server.read(str_recv_buffer, 10) == false) {
        printf("tcp_server.read(%s, 10) failed.\n", str_recv_buffer.c_str());
        return false;
    }
    // logfile.write("接收: %s\n", str_recv_buffer.c_str());
    return true;
}

// 处理客户端的登录报文，登录的作用是得到客户端程序的运行参数  
bool clientLogin() {
    // 接收客户端的登录报文
    if (tcp_server.read(str_recv_buffer, 10) == false) {
        logfile.write("tcp_server.read() failed.\n");
        return false;
    }
    // logfile.write("str_recv_buffer = %s\n", str_recv_buffer.c_str());

    // 解析客户端的登录报文，不需要对参数做合法性判断，客户端已经判断过了
    memset(&st_arg, 0, sizeof(StArg));
    getxmlbuffer(str_recv_buffer, "clienttype", st_arg.client_type);
    getxmlbuffer(str_recv_buffer, "clientpath", st_arg.client_path);
    getxmlbuffer(str_recv_buffer, "serverpath", st_arg.server_path);
    getxmlbuffer(str_recv_buffer, "serverpathbak", st_arg.server_pathbak);
    getxmlbuffer(str_recv_buffer, "ischild", st_arg.is_child);
    getxmlbuffer(str_recv_buffer, "ptype", st_arg.ptype);
    getxmlbuffer(str_recv_buffer, "matchname", st_arg.matchname);

    // 客户端程序不上传文件时，发送心跳报文的周期，保证 TCP 连接一直不断开
    getxmlbuffer(str_recv_buffer, "timetvl", st_arg.timetvl);
    // 客户端程序的进程心跳超时时间   
    getxmlbuffer(str_recv_buffer, "timeout", st_arg.timeout);
    // 客户端程序的进程名
    getxmlbuffer(str_recv_buffer, "pname", st_arg.pname);

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

    logfile.write("%s login %s.\n", tcp_server.getip(), str_send_buffer.c_str());
    return true;
}

// 处理客户端上传文件的主函数
void recvFilesMain() {
    while (true) {
        // 子进程每处理一次上传的文件或者心跳报文，更新进程心跳
        pactive.updateAtime();

        // 接收客户端的报文
        if (tcp_server.read(str_recv_buffer, st_arg.timetvl + 10) == false) {
            logfile.write("tcp_server.read() failed.\n");
            return;
        }
        // logfile.write("str_recv_buffer = %s\n", str_recv_buffer.c_str());

        // 处理 tcp 长连接心跳机制的报文
        if (str_recv_buffer == "<activetest>ok</activetest>") {
            str_send_buffer = "ok";
            // logfile.write("str_send_buffer = %s\n", str_send_buffer.c_str());
            if (tcp_server.write(str_send_buffer) == false) {
                logfile.write("tcp_server.write() failed.\n");
                return;
            }
        }

        // 处理上传文件的请求报文，如果 string::find() 返回 string::npos 说明没有匹配到要找的字符串
        if (str_recv_buffer.find("<filename>") != string::npos) {
            // 解析上传文件请求报文的 xml
            string client_filename;     // 客户端待上传的文件名
            string m_time;              // 文件的时间
            int file_size = 0;          // 文件大小
            getxmlbuffer(str_recv_buffer, "filename", client_filename);
            getxmlbuffer(str_recv_buffer, "mtime", m_time);
            getxmlbuffer(str_recv_buffer, "size", file_size);

            // 接收文件的内容
            // st_arg.client_path = /tmp/client
            // st_arg.server_path = /tmp/server
            // 客户端的文件名是: client_filename = /tmp/client/aaa/1.txt
            // 服务端的文件名是: server_filename = /tmp/server/aaa/1.txt
            string server_filename;     // 服务端的文件名
            server_filename = client_filename;
            replacestr(server_filename, st_arg.client_path, st_arg.server_path, false);
            logfile.write("recv %s(%d)...", server_filename.c_str(), filesize);
            if (recvFilesContent(server_filename, m_time, file_size) == true) {
                logfile << "ok.\n";
                sformat(str_send_buffer, "<filename>%s</filename><result>ok</result>", client_filename.c_str());
            }
            else {
                logfile << "failed.\n";
                sformat(str_send_buffer, "<filename>%s</filename><result>failed</result>", client_filename.c_str());
            }

            // 把确认报文发送给客户端
            // logfile.write("str_send_buffer = %s\n", str_send_buffer.c_str());
            if (tcp_server.write(str_send_buffer) == false) {
                logfile.write("tcp_server.write() failed.\n");
                return;
            }
        }
    }
}

// 接收客户端上传文件的内容
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
        if (tcp_server.read(buffer, on_read) == false) {
            logfile.write("tcp_server.read() failed.\n");
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
    // 文件的修改时间与客户端一致
    setmtime(filename, m_time);

    return true;
}

// 处理客户端下载文件的主函数
void sendFilesMain() {

    // pactive.addProcInfo(st_arg.timeout, st_arg.pname);

    bool bcontinue = true;   // 如果调用 tcpPutFiles() 发送了文件，bcontinue为true，否则为false

    while (true)
    {
        // 调用文件下载的主函数，执行一次文件下载的任务。
        if (tcpPutFiles(bcontinue) == false) {
            logfile.write("tcpPutFiles() failed.\n");
            return;
        }

        if (bcontinue == false) {
            sleep(st_arg.timetvl);

            if (activeTest() == false) {
                break;
            }
        }

        pactive.updateAtime();
    }
}

// 文件下载的主函数，执行一次文件下载的任务
bool tcpPutFiles(bool& bcontinue) {
    // 标记是否有上传文件的任务
    bcontinue = false;

    cdir dir;

    // 打开 st_arg.client_path 目录
    if (dir.opendir(st_arg.server_path, st_arg.matchname, 10000, st_arg.is_child) == false) {
        logfile.write("dir.opendir(%s) failed.\n", st_arg.server_path);
        return false;
    }

    // 未收到客户端确认报文的文件数量，发送了一个文件就加 1，接收了一个回应就减 1
    int delayed = 0;

    // 遍历目录中的每个文件
    while (dir.readdir()) {
        bcontinue = true;
        // 把文件名、修改时间、文件大小组成报文段，发送给客户端
        sformat(str_send_buffer, "<filename>%s</filename><mtime>%s</mtime><size>%d</size>",
            dir.m_ffilename.c_str(), dir.m_mtime.c_str(), dir.m_filesize);
        // logfile.write("str_send_buffer = %s\n", str_send_buffer.c_str());
        if (tcp_server.write(str_send_buffer) == false) {
            logfile.write("tcp_client.write() failed.\n");
            return false;
        }

        // 发送文件内容
        logfile.write("send %s(%d)...", dir.m_ffilename.c_str(), dir.m_filesize);
        if (sendFile(dir.m_ffilename, dir.m_filesize) == false) {
            logfile << "failed.\n";
            tcp_server.closeclient();
            return false;
        }
        else {
            logfile << "ok.\n";
            ++delayed;
        }

        // 上传给客户端一个文件，更新进程心跳
        pactive.updateAtime();

        while (delayed > 0) {
            // 接收客户端的确认报文
            if (tcp_server.read(str_recv_buffer, -1) == false) {
                break;
            }
            // logfile.write("str_recv_buffer = %s\n", str_recv_buffer.c_str());

            // 处理客户端的确认报文，删除本地（服务端）文件或把本地文件移动到备份目录
            ackMessage(str_recv_buffer);
            --delayed;
        }
    }

    // 继续接收客户端的确认报文（防止确认报文没有接收完）
    while (delayed > 0) {
        // 接收客户端的确认报文
        if (tcp_server.read(str_recv_buffer, 10) == false) {
            break;
        }
        // logfile.write("str_recv_buffer = %s\n", str_recv_buffer.c_str());

        // 处理客户端的确认报文，删除本地（服务端）文件或把本地文件移动到备份目录
        ackMessage(str_recv_buffer);
        --delayed;
    }

    return true;
}

// 把文件的内容发送给客户端（当客户端需要下载文件）
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

        // 把读取到的数据发送给客户端
        if (tcp_server.write(buffer, on_read) == false) {
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
        // 生成转存后的备份目录文件名，例如：/tmp/server/2.xml /tmp/serverbak/2.xml
        string bak_filename = filename;
        replacestr(bak_filename, st_arg.server_path, st_arg.server_pathbak, false);     // 最后一个参数为 false
        if (renamefile(filename, bak_filename) == false) {
            logfile.write("renamefile(%s,%s) failed.\n", filename.c_str(), bak_filename.c_str());
            return false;
        }
    }
    return true;
}

