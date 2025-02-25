// 程序名：client.cpp，模拟浏览器，向网站发送请求报文。
#include "_public.h"
using namespace std;
using namespace idc;

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        cout << "Using:./client 服务端的IP 服务端的端口\nExample:./client www.weather.com.cn 80\n\n";
        return -1;
    }

    // 第1步：创建客户端的socket。  
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket"); return -1;
    }

    // 第2步：向服务器发起连接请求。 
    struct hostent* h;    // 用于存放服务端IP的结构体。
    if ((h = gethostbyname(argv[1])) == 0)  // 把字符串格式的IP转换成结构体。
    {
        cout << "gethostbyname failed.\n" << endl; close(sockfd); return -1;
    }
    struct sockaddr_in servaddr;              // 用于存放服务端IP和端口的结构体。
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy(&servaddr.sin_addr, h->h_addr, h->h_length); // 指定服务端的IP地址。
    servaddr.sin_port = htons(atoi(argv[2]));         // 指定服务端的通信端口。

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)  // 向服务端发起连接清求。
    {
        perror("connect"); close(sockfd); return -1;
    }

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, \
        "GET / HTTP/1.1\r\n"\
        "Host: %s:%s\r\n"\
        "\r\n", argv[1], argv[2]);
    if (send(sockfd, buffer, strlen(buffer), 0) <= 0)
    {
        perror("send"); close(sockfd); return -1;
    }
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        if (recv(sockfd, buffer, sizeof(buffer), 0) <= 0)
        {
            perror("recv"); close(sockfd); return -1;
        }

        cout << buffer << endl;
    }
    close(sockfd);
}
