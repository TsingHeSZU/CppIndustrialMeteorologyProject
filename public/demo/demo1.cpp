/*
 * 程序名：demo1.cpp，此程序用于演示socket的客户端
*/
#include "../_public.h"
using namespace std;
using namespace idc;
 
int main(int argc,char *argv[])
{
    if (argc!=3)
    {
        cout << "Using:./demo1 服务端的IP 服务端的端口\nExample:./demo1 192.168.150.128 5005\n\n"; 
        return -1;
    }

    // 第1步：创建客户端的socket。  
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd==-1)
    {
        perror("socket"); return -1;
    }
 
    // 第2步：向服务器发起连接请求。 
    struct hostent* h;    // 用于存放服务端IP的结构体。
    if ( (h = gethostbyname(argv[1])) == 0 )  // 把字符串格式的IP转换成结构体。
    {   
        cout << "gethostbyname failed.\n" << endl; close(sockfd); return -1;
    }
    struct sockaddr_in servaddr;              // 用于存放服务端IP和端口的结构体。
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy(&servaddr.sin_addr,h->h_addr,h->h_length); // 指定服务端的IP地址。
    servaddr.sin_port = htons(atoi(argv[2]));         // 指定服务端的通信端口。
  
    if (connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr))!=0)  // 向服务端发起连接清求。
    {   
        perror("connect"); close(sockfd); return -1; 
    }
  
    // 第3步：与服务端通讯。
    for (int ii=0;ii<10;ii++)  
    {
        char buffer[1024];
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"这是第%d个超级女生，编号%03d。",ii+1,ii+1);  // 生成请求报文内容。
        // if (send(sockfd,buffer,strlen(buffer),0)<=0) break;
        if (tcpwrite(sockfd,buffer)==false) break;
        cout << "发送：" << buffer << endl;
        // sleep(1);
    }
 
    // 第4步：关闭socket，释放资源。
    close(sockfd);
}