// 程序名：server2.cpp，简易的数据访问接口，接收http请求，解析请求参数，从Oracle的T_ZHOBTMIND1表中查询数据，返回给客户端。
#include "_public.h"
#include "_ooci.h"
using namespace std;
using namespace idc;

// 从GET请求中获取参数的值：strget-GET请求报文的内容；name-参数名；value-参数值；len-参数值的长度。
bool getvalue(const string& strget, const string& name, string& value, const int len);

// 解析GET请求中的参数，从T_ZHOBTMIND1表中查询数据，返回给客户端。
bool senddata(const int sockfd, const char* strget);

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cout << "Using:./server2 通讯端口\nExample:./server2 8080\n\n"; return -1;
    }

    // 第1步：创建服务端的socket。 
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("socket"); return -1;
    }

    int opt = 1; unsigned int len = sizeof(opt);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);

    // 第2步：把服务端用于通信的IP和端口绑定到socket上。 
    struct sockaddr_in servaddr;          // 用于存放服务端IP和端口的数据结构。
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;        // 指定协议。
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 服务端任意网卡的IP都可以用于通讯。
    servaddr.sin_port = htons(atoi(argv[1]));     // 指定通信端口，普通用户只能用1024以上的端口。
    // 绑定服务端的IP和端口。
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("bind"); close(listenfd); return -1;
    }

    // 第3步：把socket设置为可连接（监听）的状态。
    if (listen(listenfd, 5) != 0)
    {
        perror("listen"); close(listenfd); return -1;
    }

    // 第4步：受理客户端的连接请求，如果没有客户端连上来，accept()函数将阻塞等待。
    int clientfd = accept(listenfd, 0, 0);
    if (clientfd == -1)
    {
        perror("accept"); close(listenfd); return -1;
    }

    cout << "客户端已连接。\n";

    // 接收客户端的http请求报文。
    char recvbuffer[1024];
    memset(recvbuffer, 0, sizeof(recvbuffer));
    if ((recv(clientfd, recvbuffer, sizeof(recvbuffer), 0)) <= 0) return -1;
    cout << recvbuffer << endl;

    // 把响应报文的状态行和头部信息发送给http客户端。
    char sendbuffer[1024];
    memset(sendbuffer, 0, sizeof(sendbuffer));
    sprintf(sendbuffer, \
        "HTTP/1.1 200 OK\r\n"
        "Server: server2\r\n"
        "Content-Type: text/html\r\n"
        "\r\n");
    send(clientfd, sendbuffer, strlen(sendbuffer), 0);    // 把响应报文的状态行和头部信息发送给http客户端。

    senddata(clientfd, recvbuffer);                              // 解析GET请求中的参数，从T_ZHOBTMIND1表中查询数据，返回给客户端。

    // sleep(10);

    // 第6步：关闭socket，释放资源。
    close(listenfd);   // 关闭服务端用于监听的socket。
    close(clientfd);   // 关闭客户端连上来的socket。
}

// 从GET请求中获取参数的值：strget-GET请求报文的内容；name-参数名；value-参数值；len-参数值的长度。
bool getvalue(const string& strget, const string& name, string& value, const int len)
{
    // http://192.168.150.128:8080/api?username=wucz&passwd=wuczpwd
    // GET /api?username=wucz&passwd=wuczpwd HTTP/1.1
    // Host: 192.168.150.128:8080
    // Connection: keep-alive
    // Upgrade-Insecure-Requests: 1
    // .......
    int startp = strget.find(name);                                          // 在请求行中查找参数名的位置。
    if (startp == string::npos) return false;

    int endp = strget.find("&", startp);                                    // 从参数名的位置开始，查找&符号。
    if (endp == string::npos) endp = strget.find(" ", startp);     // 如果是最后一个参数，没有找到&符号，那就查找空格。

    if (endp == string::npos) return false;

    // 从请求行中截取参数的值。
    int itmplen = endp - startp - (name.length() + 1);
    if ((len > 0) && (len < itmplen)) itmplen = len;
    value = strget.substr(startp + (name.length() + 1), itmplen);

    return true;
}

// 解析GET请求中的参数，从T_ZHOBTMIND1表中查询数据，返回给客户端。
bool senddata(const int sockfd, const char* strget)
{
    // 解析URL中的参数。
    // 权限控制：用户名和密码。
    // 接口名：访问数据的种类。
    // 查询条件：设计接口的时候决定。                                           可读性不好，可扩展性也不好。
    // http://192.168.150.128:8080/api?wucz&wuczpwd&getZHOBTMIND1&51076&20211024094318&20211024114020
    // http://47.107.28.141:8080/api?username=utopianyouth&passwd=123&intername=getZHOBTMIND1&obtid=51076&begintime=20250224000000&endtime=20250225000000

    string username, passwd, intername, obtid, begintime, endtime;

    // 1）从请求行中把url后面的参数解析出来。
    getvalue(strget, "username", username, 30);    // 获取用户名。
    getvalue(strget, "passwd", passwd, 30);            // 获取密码。
    getvalue(strget, "intername", intername, 30);   // 获取接口名。
    getvalue(strget, "obtid", obtid, 10);                  // 获取站点代码。
    getvalue(strget, "begintime", begintime, 20);   // 获取起始时间。
    getvalue(strget, "endtime", endtime, 20);         // 获取结束时间。

    cout << "username=" << username << endl;
    cout << "passwd=" << passwd << endl;
    cout << "intername=" << intername << endl;
    cout << "obtid=" << obtid << endl;
    cout << "begintime=" << begintime << endl;
    cout << "endtime=" << endtime << endl;

    // 连接数据库。
    connection conn;
    conn.connecttodb("idc/idcpwd", "Simplified Chinese_China.AL32UTF8");

    // 判断用户名/密码和接口名是否合法。

    // 准备查询数据的SQL。
    sqlstatement stmt(&conn);
    stmt.prepare(
        "select '<obtid>'||obtid||'</obtid>'||'<ddatetime>'||to_char(ddatetime,'yyyy-mm-dd hh24:mi:ss')||'</ddatetime>'||'<t>'||t||'</t>'||'<p>'||p||'</p>'||'<u>'||u||'</u>'||'<keyid>'||keyid||'</keyid>'||'<endl/>' "
        "from T_ZHOBTMIND1 where obtid=:1 and ddatetime>to_date(:2,'yyyymmddhh24miss') and ddatetime<to_date(:3,'yyyymmddhh24miss')");
    char strxml[1001];  // 存放SQL语句的结果集。
    stmt.bindout(1, strxml, 1000);
    stmt.bindin(1, obtid, 10);
    stmt.bindin(2, begintime, 14);
    stmt.bindin(3, endtime, 14);

    stmt.execute();   // 执行查询数据的SQL。

    writen(sockfd, "<data>\n", strlen("<data>\n"));      // 返回xml的头部标签。

    while (true)
    {
        memset(strxml, 0, sizeof(strxml));
        if (stmt.next() != 0) break;

        strcat(strxml, "\n");                                     // 注意加上换行符。
        writen(sockfd, strxml, strlen(strxml));          // 返回xml的每一行。
    }

    writen(sockfd, "</data>\n", strlen("</data>\n"));  // 返回xml的尾部标签。

    return true;
}