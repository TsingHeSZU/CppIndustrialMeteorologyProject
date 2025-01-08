/*
 *  程序名：demo44.cpp，此程序演示采用开发框架的ctcpclient类传输文本数据（网络通讯的客户端） 
 *  作者：吴从周
*/
#include "../_public.h"
using namespace std;
using namespace idc;

int main(int argc,char *argv[])
{
    if (argc!=3)
    {
        printf("Using:./demo44 ip port\n");
        printf("Sample:./demo44 192.168.150.128 5005\n");
        return -1;
    }

    ctcpclient tcpclient;
    if (tcpclient.connect(argv[1],atoi(argv[2]))==false)     // 向服务端发起连接请求。
    {
        printf("tcpclient.connect(%s,%s) failed.\n",argv[1],argv[2]); return -1;
    }

    string sendbuf,recvbuf;

    for (int ii=0;ii<10;ii++)
    {
        sendbuf=sformat("这是第%d个超级女生。",ii);

        if (tcpclient.write(sendbuf)==false)        // 向服务端发送请求报文。
        {
            printf("tcpclient.write() failed.\n"); break;
        }
        cout << "发送：" << sendbuf << endl;

        sleep(1);

        if (tcpclient.read(recvbuf)==false)         // 接收服务端的回应报文。
        {
            printf("tcpclient.read() failed.\n"); break;
        }
        cout << "接收：" << recvbuf << endl;
    }
}