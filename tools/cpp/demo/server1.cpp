// 程序名：server1.cpp，接收浏览器发过来的http请求报文，然后返回一个网页文件或数据文件。
#include "_public.h"
using namespace std;
using namespace idc;

// 把文件的内容发送给对端。
bool sendfile(int sock,const string &filename);
 
int main(int argc,char *argv[])
{
    if (argc!=3)
    {
        cout << "Using:./server2 通讯端口 文件名\nExample:./server2 80 index.html\n\n"; return -1;
    }

    // 第1步：创建服务端的socket。 
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd==-1) 
    { 
        perror("socket"); return -1; 
    }

    int opt = 1; unsigned int len = sizeof(opt);
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,len);
  
    // 第2步：把服务端用于通信的IP和端口绑定到socket上。 
    struct sockaddr_in servaddr;          // 用于存放服务端IP和端口的数据结构。
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;        // 指定协议。
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 服务端任意网卡的IP都可以用于通讯。
    servaddr.sin_port = htons(atoi(argv[1]));     // 指定通信端口，普通用户只能用1024以上的端口。
    // 绑定服务端的IP和端口。
    if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) != 0 )
    { 
        perror("bind"); close(listenfd); return -1; 
    }
 
    // 第3步：把socket设置为可连接（监听）的状态。
    if (listen(listenfd,5) != 0 ) 
    { 
        perror("listen"); close(listenfd); return -1; 
    }
 
    // 第4步：受理客户端的连接请求，如果没有客户端连上来，accept()函数将阻塞等待。
    int clientfd=accept(listenfd,0,0);
    if (clientfd==-1)
    {
        perror("accept"); close(listenfd); return -1; 
    }

    cout << "客户端已连接。\n";
 
    char buffer[1024];
    memset(buffer,0,sizeof(buffer));

    if ( (recv(clientfd,buffer,sizeof(buffer),0))<=0) return -1;

    cout << buffer << endl;

    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,\
        "HTTP/1.1 200 OK\r\n"
        "Server: server1\r\n"
        "Content-Length: %d\r\n"
        "Content-Type: text/html\r\n"
        "\r\n",filesize(argv[2]));
    send(clientfd,buffer,strlen(buffer),0);    // 把响应报文的状态行和头部信息发送给http客户端。

    sendfile(clientfd,argv[2]);                      // 把文件的内容发送给http客户端。

    // sleep(10);
 
    // 第6步：关闭socket，释放资源。
    close(listenfd);   // 关闭服务端用于监听的socket。
    close(clientfd);   // 关闭客户端连上来的socket。
}

// 把文件的内容发送给http客户端。
bool sendfile(int sock,const string &filename)
{
    int  onread=0;         // 每次打算从文件中读取的字节数。 
    char buffer[1000];   // 存放读取数据的buffer，buffer的大小可参考硬盘一次读取数据量（4K为宜）。
    int  totalbytes=0;    // 从文件中已读取的字节总数。
    cifile ifile;                 // 读取文件的对象。

    int totalsize=filesize(filename);

    // 必须以二进制的方式操作文件。
    if (ifile.open(filename,ios::in|ios::binary)==false) return false;

    while (true)
    {
        memset(buffer,0,sizeof(buffer));

        // 计算本次应该读取的字节数，如果剩余的数据超过1000字节，就读1000字节。
        if (totalsize-totalbytes>1000) onread=1000;
        else onread=totalsize-totalbytes;

        // 从文件中读取数据。
        ifile.read(buffer,onread);   

        // 把读取到的数据发送给对端。
        send(sock,buffer,onread,0);

        // 计算文件已读取的字节总数，如果文件已读完，跳出循环。
        totalbytes=totalbytes+onread;

        if (totalbytes==totalsize) break;
    }

    return true;
}