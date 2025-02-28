/*
    数据访问接口模块
    测试 url
    - 47.107.28.141:5088/api?username=utopianyouth&passwd=123&intername=getzhobtcode
    - 47.107.28.141:5088/api?username=tsing&passwd=123&intername=getzhobtcode
    - 47.107.28.141:5088/api?username=tsing&passwd=123&intername=getzhobtmind1&obtid=58150
    - 47.107.28.141:5088/api?username=utopianyouth&passwd=123&intername=getzhobtmind2&begintime=20250224000000&endtime=20250225000000
    - 47.107.28.141:5088/api?username=tsing&passwd=123&intername=getzhobtmind3&obtid=51076&begintime=20250224000000&endtime=20250225000000
    - wget "localhost:5088/api?username=tsing&passwd=123&intername=getzhobtmind3&obtid=51076&begintime=20250224000000&endtime=20250225000000" -0 abc.xml
*/
#include"_public.h"
#include"_ooci.h"
using namespace idc;

// 程序日志文件
clogfile logfile;

/*
    从GET请求中获取参数的值
    - strget：请求报文的内容
    - name：参数名
    - value：参数值
*/
bool getvalue(const string& strget, const string& name, string& value);

// 初始化服务端的监听端口
int initServer(const int port);

// 客户端结构体
typedef struct StClient {
    int clientatime = 0;        // 客户端最后一次活动的时间
    string recvbuffer;          // 客户端的接收缓冲区
    string sendbuffer;          // 客户端的发送缓冲区
}StClient;

// 接收&发送队列的结构体
typedef struct StRecvSendMsg {
    int commucation_fd = 0;     // 客户端与服务器通信用的 fd
    string message;             // 接收&发送的报文内容

    StRecvSendMsg(int in_commucation_fd, string& in_message) :
        commucation_fd(in_commucation_fd), message(in_message) {
        logfile.write("StRecvSendMsg(){};\n");
    };

}StRecvSendMsg;

// 线程类
class AA {
private:
    // 接收队列，底层容器用 deque，使用智能指针可以避免内存拷贝，使用移动语义
    queue<shared_ptr<StRecvSendMsg>> m_recv_queue;
    mutex m_mutex_recv_queue;               // 接收队列的互斥锁
    condition_variable m_cond_recv_queue;   // 接收队列的条件变量
    queue<shared_ptr<StRecvSendMsg>> m_send_queue;      // 发送队列，底层容器用 deque
    mutex m_mutex_send_queue;       // 发送队列的互斥锁
    int m_send_pipe[2] = { 0 };     // 工作线程通知发送线程的匿名管道
    unordered_map<int, StClient> client_map;            // 存放客户端对象的哈希表，俗称状态机
    /*
        m_exit 变量的读写操作不会被其它线程中断，属于原子操作
        如果 m_exit == true, 工作线程和发送线程将退出
    */
    atomic_bool m_exit;

public:
    // 主进程通知接收线程退出的匿名管道，主进程要用到该成员，所以声明为 public
    int m_recv_pipe[2] = { 0 };

    AA() {
        // 创建&初始化匿名管道
        pipe(this->m_send_pipe);
        pipe(this->m_recv_pipe);
        this->m_exit = false;
    }

    /*
        接收线程主函数
        - listenport：监听端口
    */
    void recvFunc(int listenport) {
        // 初始化服务端用于监听的 fd
        int listenfd = initServer(listenport);
        if (listenfd < 0) {
            logfile.write("recv thread: initServer(%d) failed.\n", listenfd);
            return;
        }

        // 创建 epoll 句柄
        int epollfd = epoll_create1(0);
        // 声明 epoll 事件的数据结构
        struct epoll_event ev;

        // 为监听的 listenfd 准备读事件
        ev.events = EPOLLIN;        // 读事件
        ev.data.fd = listenfd;      // 指定事件的自定义数据，会随着 epoll_wait() 返回的事件一并返回
        epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);   // 把监听用的 socket 事件加入到 epollfd 中

        // 把接收主进程通知的管道加入 epoll
        ev.data.fd = this->m_recv_pipe[0];
        ev.events = EPOLLIN;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev);

        struct epoll_event evs[10]; // 存放 epoll 返回的事件

        // 进入事件循环
        while (true) {
            // 等待监视的 fd 有事件发生，返回发生事件的 fd 个数
            int infds = epoll_wait(epollfd, evs, 10, -1);

            // 返回失败
            if (infds < 0) {
                logfile.write("recv thread: epoll() failed.\n");
                return;
            }

            // 遍历 epoll 返回的已发生事件的数组 evs
            for (int i = 0;i < infds;++i) {
                logfile.write("recv thread: have occured event fd = %d, events = %d.\n", evs[i].data.fd, evs[i].events);

                // 如果发生的事件是 listenfd，表示有新的客户端连接上来
                if (evs[i].data.fd == listenfd) {
                    struct sockaddr_in clientaddr;
                    socklen_t len = sizeof(clientaddr);
                    int communication_fd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);

                    // 把通信用的 communication_fd 设置为非阻塞
                    fcntl(communication_fd, F_SETFL, fcntl(communication_fd, F_GETFD, 0) | O_NONBLOCK);

                    // inet_ntoa 得到客户端连接的 ip 地址
                    logfile.write("recv thread: accept client ip = %s, communication_fd = %d ok.\n",
                        inet_ntoa(clientaddr.sin_addr), communication_fd);

                    // 为新的客户端连接准备读事件，并添加到 epoll 中
                    ev.data.fd = communication_fd;
                    ev.events = EPOLLIN;
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, communication_fd, &ev);
                    continue;
                }

                // 发生的事件是主进程通知接收线程退出的匿名管道读端
                if (evs[i].data.fd == this->m_recv_pipe[0]) {
                    logfile.write("recv thread: exiting.\n");

                    // 把退出的原子变量设置为 true
                    this->m_exit = true;
                    // 通知全部的工作线程退出
                    this->m_cond_recv_queue.notify_all();
                    // 通知发送线程退出
                    write(this->m_send_pipe[1], (char*)"o", 1);
                    return;
                }

                /*
                    如果是客户端连接的 socket 有事件，分两种情况
                    - 客户端有报文发过来
                    - 客户端连接已断开
                */
                if (evs[i].events & EPOLLIN) {
                    // 判断是否为读事件
                    char buffer[5000];  // 存放从接收缓冲区中读取的数据（客户端发过来）
                    int buflen = 0;     // 从接收缓冲区读取数据的大小

                    /*
                        读取客户端的请求报文，由于使用了 epoll，只有对应的 fd 有数据时，才会执行 recv()
                        这里需要分析一下 recv() 返回值的情况
                        - 阻塞模式下
                            - 大于0：表示正常接收到了客户端发送的数据
                            - 等于0：表示客户端发送了 FIN 包，请求断开 tcp 连接
                            - 小于0：表示 tcp 通信出现错误
                            - 客户端没有发送数据时，recv() 函数阻塞
                        - 非阻塞模式下
                            - 大于0：表示正常接收到了客户端发送的数据
                            - 等于0：表示客户端发送了 FIN 包，请求断开 tcp 连接
                            - -1：表示客户端没有发送数据
                            - other：表示 tcp 通信出现错误
                    */
                    buflen = recv(evs[i].data.fd, buffer, sizeof(buffer), 0);
                    if (buflen <= 0) {
                        // 连接已断开
                        logfile.write("recv thread: cilent(%d) disconnected.\n", evs[i].data.fd);
                        // 关闭客户端的连接（即与客户端通信的文件描述符）
                        close(evs[i].data.fd);
                        // 从状态机机中删除客户端
                        this->client_map.erase(evs[i].data.fd);
                        continue;
                    }

                    logfile.write("recv thread: recv(%d), %d bytes.\n", evs[i].data.fd, buflen);

                    // 把读取到的数据追加到客户端通信 fd 的缓冲区
                    this->client_map[evs[i].data.fd].recvbuffer.append(buffer, buflen);

                    // 如果接收到的报文内容以 "\r\n\r\n" 结束，表示已经是一个完整的 http 请求报文了
                    if (this->client_map[evs[i].data.fd].recvbuffer.compare(
                        this->client_map[evs[i].data.fd].recvbuffer.length() - 4, 4, "\r\n\r\n") == 0) {
                        logfile.write("recv thread: receive an integrated request messagse.\n");

                        // 把完整的请求报文入队，交给工作线程
                        this->inRecvQueue((int)evs[i].data.fd, this->client_map[evs[i].data.fd].recvbuffer);

                        // 清空客户端 fd 中的 buffer
                        this->client_map[evs[i].data.fd].recvbuffer.clear();
                    }
                    else {
                        // 限制接收缓冲区大小为 1000 字节，视 http 请求报文长度改变
                        if (this->client_map[evs[i].data.fd].recvbuffer.size() > 1000) {
                            close(evs[i].data.fd);      // 关闭客户端的连接
                            // 从状态机中删除客户端
                            this->client_map.erase(evs[i].data.fd);

                            // 可以考虑把客户端的 ip 加入黑名单

                        }
                    }

                    // 更新客户端的活动时间
                    this->client_map[evs[i].data.fd].clientatime = time(0);
                }
            }
        }
    }

    /*
        把客户端的 communication_fd 和请求报文放入接收队列，等待工作线程处理
        - communication_fd: 服务器接收客户端的 tcp 连接请求后，创建用来通信的 fd
        - message: 客户端的 http 请求报文
    */
    void inRecvQueue(int communication_fd, string& message) {
        // 创建接收 http 请求报文的对象
        shared_ptr<StRecvSendMsg> ptr = make_shared<StRecvSendMsg>(communication_fd, message);

        // 申请加锁
        lock_guard<mutex> lock(this->m_mutex_recv_queue);

        // 把接收了 http 请求的 StRecvSendMsg 扔到接收队列中
        this->m_recv_queue.push(ptr);

        // 通知工作线程处理接收队列中的报文
        this->m_cond_recv_queue.notify_one();
    }

    /*
        工作线程主函数，处理接收队列中的请求报文
        - id: 线程编号
    */
    void workFunc(int id) {
        connection conn;

        if (conn.connecttodb("idc/idcpwd", "Simplified Chinese_China.AL32UTF8") != 0) {
            logfile.write("connect database(idc/idcpwd) failed.\n");
            return;
        }

        while (true) {
            shared_ptr<StRecvSendMsg> ptr;

            // 用 {} 围起来的变量，只在该作用域内有效
            {
                // 把互斥锁转换成 unique_lock<mutex>，并申请加锁
                unique_lock<mutex> lock(this->m_mutex_recv_queue);

                // 如果队列空，进入循环，否则直接处理数据，必须用 while，不能用 if
                while (this->m_recv_queue.empty()) {
                    // 等待生产者的唤醒信号
                    this->m_cond_recv_queue.wait(lock);

                    if (this->m_exit == true) {
                        logfile.write("work thread(%d): is exiting.\n", id);
                        return;
                    }
                }

                // 队列不为空，从接收队列中取出一个客户端 http 请求的结构体对象
                ptr = this->m_recv_queue.front();
                this->m_recv_queue.pop();

            }

            // 处理出队的客户端 http 请求对象
            logfile.write("work thread(%d): client communication_fd = %d, message = %s\n",
                id, ptr->commucation_fd, ptr->message.c_str());

            // 增加处理客户端请求报文的代码（解析请求报文、判断权限、执行查询数据的 sql 语句，生成响应报文）
            string sendbuf;
            bizMain(conn, ptr->message, sendbuf);
            string message = sformat(
                "HTTP/1.1 200 OK\r\n"\
                "Server: webserver\r\n"\
                "Content-Type: text/html;charset=utf-8\r\n"
            ) + sformat("Content-Length:%d\r\n\r\n", sendbuf.size()) + sendbuf;

            logfile.write("work thread(%d) respose: communication_fd = %d, message = %s\n", id,
                ptr->commucation_fd, ptr->message.c_str());

            // 把客户端与服务器通信的 fd 和响应报文加入发送队列
            this->inSendQueue(ptr->commucation_fd, message);
        }
    }

    /*
        把客户端与服务器通信的 fd 和响应报文加入发送队列
        - communication_fd: 客户端与服务器通信的文件描述符
        - message: 客户端的响应报文
    */
    void inSendQueue(int communication_fd, string& message) {
        {
            shared_ptr<StRecvSendMsg> ptr = make_shared<StRecvSendMsg>(communication_fd, message);
            // 申请加锁
            lock_guard<mutex> lock(this->m_mutex_send_queue);

            this->m_send_queue.push(ptr);
        }

        // 通知发送线程处理发送队列中的数据，往管道中写数据
        write(this->m_send_pipe[1], (char*)"o", 1);
    }

    /*
        发送线程主函数，把发送队列中的数据发送给客户端

        工作线程如何通知发送线程有内容需要发送：
        - 信号：epoll 可以监视信号，工作线程用信号通知发送线程
        - tcp：工作线程与发送线程创建一个 tcp 连接，工作线程用这个 tcp 连接通知发送线程
        - 匿名管道：由于工作线程和发送线程属于同一个进程，所以两个线程的通信可以使用匿名管道（推荐且简单）

        不能使用条件变量：条件变量也会阻塞，这就会导致线程会存在两个阻塞的地方
    */
    void sendFunc() {
        // 创建 epoll 句柄
        int epollfd = epoll_create1(0);
        // 声明事件的数据结构
        struct epoll_event ev;

        // 把发送队列管道的读端加入 epoll
        ev.data.fd = this->m_send_pipe[0];
        ev.events = EPOLLIN;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev);

        // 存放 epoll 返回的事件
        struct epoll_event evs[10];

        // 进入事件循环
        while (true) {
            // 等待监视的 fd 有事件发生
            int infds = epoll_wait(epollfd, evs, 10, -1);
            // 返回失败
            if (infds < 0) {
                logfile.write("send thread: epoll() failed.\n");
                return;
            }
            // 遍历 epoll 返回的已发生事件的数组 evs
            for (int i = 0;i < infds;++i) {
                logfile.write("send thread: have occured event fd = %d, events = %d.\n", evs[i].data.fd, evs[i].events);

                // 如果发生事件的是管道，表示发送队列中有报文需要发送
                if (evs[i].data.fd == this->m_send_pipe[0]) {
                    if (this->m_exit == true) {
                        logfile.write("send thread: is exiting.\n");
                        return;
                    }

                    char cc;
                    // 读取管道中的数据，只有一个字符，不关心其内容
                    read(this->m_send_pipe[0], &cc, 1);

                    shared_ptr<StRecvSendMsg> ptr;
                    // 申请加锁
                    lock_guard<mutex> lock(this->m_mutex_send_queue);

                    while (this->m_send_queue.empty() == false) {
                        ptr = this->m_send_queue.front();
                        // 出队一个元素
                        this->m_send_queue.pop();

                        // 把出队的报文保存到 fd 的发送缓冲区中
                        this->client_map[ptr->commucation_fd].sendbuffer.append(ptr->message);

                        // 关注客户端的 fd 写事件
                        ev.data.fd = ptr->commucation_fd;
                        ev.events = EPOLLOUT;
                        epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev);
                    }

                    continue;
                }

                // 判断客户端的 communication_fd 是否有写事件（发送缓冲区没有满）
                if (evs[i].events & EPOLLOUT) {
                    // 把响应报文发送给客户端，string::data() 返回指向字符串首地址的指针
                    int writen = send(evs[i].data.fd, this->client_map[evs[i].data.fd].sendbuffer.data(),
                        this->client_map[evs[i].data.fd].sendbuffer.length(), 0);
                    logfile.write("send thread: send %d bytes to %d.\n", writen, evs[i].data.fd);

                    // 删除 communication_fd 缓冲区中已成功发送的数据
                    this->client_map[evs[i].data.fd].sendbuffer.erase(0, writen);

                    // 如果 communication_fd 缓冲区没有数据了，不再关心其写事件
                    if (this->client_map[evs[i].data.fd].sendbuffer.length() == 0) {
                        ev.data.fd = evs[i].data.fd;
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, ev.data.fd, &ev);
                    }
                }
            }
        }
    }

    /*
        处理客户端的请求报文，生成响应报文
        - conn：数据库连接
        - recvbuf：http 请求报文
        - sendbuf：http 响应报文的数据部分，不包括状态行和头部信息
    */
    void bizMain(connection& conn, const string& recvbuf, string& sendbuf) {
        string username, passwd, intername;
        // 解析用户名
        getvalue(recvbuf, "username", username);
        // 解析密码
        getvalue(recvbuf, "passwd", passwd);
        // 解析接口名
        getvalue(recvbuf, "intername", intername);

        // 1.验证用户名和密码是否正确
        sqlstatement stmt(&conn);
        stmt.prepare("select ip from T_USERINFO where username=:1 and passwd=:2 and rsts=1");
        string ip;
        stmt.bindin(1, username);
        stmt.bindin(2, passwd);
        stmt.bindout(1, ip, 50);
        if (stmt.execute() != 0) {
            logfile.write("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
            return;
        }

        if (stmt.next() != 0) {
            sendbuf = "<retcode>-1</retcode> <message>username or passwd error</message>";
            return;
        }

        // 2. 判断连接的 IP 地址是否在绑定 ip 地址的列表中
        if (ip.empty() == false) {
            //
        }

        // 3. 判断用户是否有访问接口的权限
        stmt.prepare("select count(*) from T_USERANDINTER "\
            "where username=:1 and intername=:2 and intername in (select intername from T_INTERCFG where rsts=1)");
        stmt.bindin(1, username);
        stmt.bindin(2, intername);
        int icount;
        stmt.bindout(1, icount);
        if (stmt.execute() != 0) {
            logfile.write("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
            return;
        }
        stmt.next();
        if (icount == 0) {
            sendbuf = "<retcode>-1</retcode> <message>user has no authority or interface is not exist</message>";
            return;
        }

        /*
            4. 获取接口的配置参数
            从接口参数配置表 T_INTERCFG 中加载接口参数
        */
        string selectsql, colstr, bindin;
        stmt.prepare("select selectsql,colstr,bindin from T_INTERCFG where intername=:1");
        stmt.bindin(1, intername);          // 接口名
        stmt.bindout(1, selectsql, 1000);   // 接口 sql
        stmt.bindout(2, colstr, 300);       // 输出列名
        stmt.bindout(3, bindin, 300);       // 接口参数
        if (stmt.execute() != 0) {
            logfile.write("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
            return;
        }

        if (stmt.next() != 0) {
            sendbuf = "<retcode>-1</retcode> <message>internal error</message>";
            return;
        }

        /*
        http://127.0.0.1:8080?username=ty&passwd=typwd&intername=getzhobtmind3&
        obtid=59287&begintime=20211024094318&endtime=20211024113920
        SQL语句: select obtid,to_char(ddatetime,'yyyymmddhh24miss'),t,p,rh,wd,ws,r,vis from T_ZHOBTMIND
                    where obtid=:1 and ddatetime>=to_date(:2,'yyyymmddhh24miss') and ddatetime<=to_date(:3,'yyyymmddhh24miss')
        colstr字段: obtid,ddatetime,t,p,rh,wd,ws,r,vis
        bindin字段: obtid,begintime,endtime
        */
        // 准备查询数据的 sql 语句
        stmt.prepare(selectsql);

        // 根据接口配置中的参数列表（bindin字段），从请求报文中解析参数的值，绑定到查询数据的 sql 语句中
        // 拆分输入参数 bindin
        ccmdstr cmdstr;
        // 拆分的结果存放到 cmdstr.m_cmdstr 容器中
        cmdstr.splittocmd(bindin, ",");

        // 声明用于存放输入参数的数组
        vector<string> invalue;
        invalue.resize(cmdstr.size());

        // 从 http 的 GET 请求报文中解析出输入参数，绑定到 sql 中
        for (int i = 0;i < cmdstr.size();++i) {
            getvalue(recvbuf, cmdstr[i].c_str(), invalue[i]);
            stmt.bindin(i + 1, invalue[i]);
        }

        // 绑定查询数据的 sql 语句的输出变量
        // 拆分 colstr，可以得到结果集的字段数
        cmdstr.splittocmd(colstr, ",");

        // 用于存放结果集的数组
        vector<string> colvalue;
        colvalue.resize(cmdstr.size());

        // 把结果集绑定到 colvalue 数组中
        for (int i = 0;i < cmdstr.size();++i) {
            stmt.bindout(i + 1, colvalue[i]);
        }

        if (stmt.execute() != 0) {
            logfile.write("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
            sformat(sendbuf, "<retcode>%d</retcode> <message>%s</message>", stmt.rc(), stmt.message());
            return;
        }

        sendbuf = "<retcode>0</retcode><message>ok</message>\n";

        sendbuf = sendbuf + "<data>\n";     // xml 内容开始的标签 <data>

        // 获取结果集，没获取一条记录，拼接 xml
        while (true) {
            // 从结果集中获取一条记录
            if (stmt.next() != 0) {
                break;
            }

            for (int i = 0;i < cmdstr.size();++i) {
                sendbuf = sendbuf + sformat("<%s>%s</%s>", cmdstr[i].c_str(),
                    colvalue[i].c_str(), cmdstr[i].c_str());
            }

            sendbuf = sendbuf + "<endl/>\n";    // 每行结束的标志
        }

        sendbuf = sendbuf + "</data>\n";    // xml 内容结尾的标签
        logfile.write("intername = %s, count= %d.\n", intername.c_str(), stmt.rpc());

        // 向接口调用日志表中写入记录...
    }
};


// 线程类对象
AA aa;

// 程序退出
void EXIT(int sig);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Using: ./webserver logfile port\n");
        printf("Sample: ./webserver /log/idc/webserver.log 5088\n");
        printf("/CppIndustrialMeteorologyProject/tools/bin/procctl 5 "\
            "/CppIndustrialMeteorologyProject/tools/bin/webserver "\
            "/log/idc/webserver.log 5088\n");
        printf("本程序是基于 HTTP 协议的数据访问接口模块, 参数解释如下:\n");
        printf("logfile: 本程序运行的日是志文件;\n");
        printf("port: 服务端口, 例如: 80, 8080 等。\n");
        return -1;
    }

    // 关闭全部的信号和 IO
    // 设置信号在 shell 状态下可用 "kill + 进程号" 正常终止这些进程
    // 但请不要用 "kill -9 + 进程号" 强行终止
    //closeioandsignal(true);
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    // 打开日志文件
    if (logfile.open(argv[1]) == false) {
        printf("open logfile(%s) failed.\n", argv[1]);
        return -1;
    }

    thread t1(&AA::recvFunc, &aa, atoi(argv[2]));   // 创建接收线程
    thread t2(&AA::workFunc, &aa, 1);               // 创建工作线程1，1是传递给线程函数的参数
    thread t3(&AA::workFunc, &aa, 2);               // 创建工作线程2
    thread t4(&AA::workFunc, &aa, 3);               // 创建工作线程3
    thread t5(&AA::sendFunc, &aa);                  // 创建发送线程

    logfile.write("started all threads.\n");

    while (true) {
        sleep(30);

        // 可以执行一些任务

    }

    return 0;
}

// 初始化服务端的监听端口
int initServer(const int port) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenfd < 0) {
        logfile.write("socket(%d) failed.\n", port);
        return -1;
    }

    int opt = 1;
    unsigned int len = sizeof(opt);

    // 监听用的端口号设置端口复用
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);

    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(port);

    // 绑定监听用的端口号
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        logfile.write("bind(%d) failed.\n", port);
        return -1;
    }

    // 监听客户端的连接
    if (listen(listenfd, 5) != 0) {
        logfile.write("listen(%d) failed.\n", port);
        close(listenfd);    // 关闭监听用的文件描述符
        return -1;
    }

    // 监听用的文件描述符非阻塞
    fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFD) | O_NONBLOCK);

    return listenfd;
}

bool getvalue(const string& strget, const string& name, string& value) {
    /*
        http://127.0.0.1/api?username=wucz&passwd=wuczpwd
        GET /api?username=wucz&passwd=wuczpwd HTTP/1.1
        Host: 192.168.150.128:8080
        Connection: keep-alive
        Upgrade-Insecure-Requests: 1
        .......
    */

    // 在请求行中查找参数名的位置
    int startp = strget.find(name);
    if (startp == string::npos) {
        return false;
    }

    // 从参数名的位置开始，查找&符号
    int endp = strget.find("&", startp);
    if (endp == string::npos) {
        //如果是最后一个参数，没有找到& 符号，那就查找空格
        endp = strget.find(" ", startp);
    }

    if (endp == string::npos) {
        return false;
    }

    // 从请求行中截取参数的值
    value = strget.substr(startp + (name.length() + 1), endp - startp - (name.length() + 1));

    return true;
}

// 程序退出
void EXIT(int sig) {
    logfile.write("program exit, sig = %d.\n", sig);
    // 通知接收线程退出
    write(aa.m_recv_pipe[1], (char*)"o", 1);
    // 让线程有足够的时间退出
    sleep(1);

    exit(0);
}