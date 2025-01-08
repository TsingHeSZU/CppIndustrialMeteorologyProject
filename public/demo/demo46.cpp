/*
 *  程序名：demo46.cpp，此程序演示采用开发框架的ctcpclient类传输二进制数据（网络通讯的客户端） 
 *  作者：吴从周
*/
#include "../_public.h"
using namespace std;
using namespace idc;

int main(int argc,char *argv[])
{
    if (argc!=3)
    {
        printf("Using:./demo46 ip port\n");
        printf("Sample./demo46 192.168.150.128 5005\n");
        return -1;
    }

    ctcpclient tcpclient;
    if (tcpclient.connect(argv[1],atoi(argv[2]))==false)     // 向服务端发起连接请求。
    {
        printf("tcpclient.connect(%s,%s) failed.\n",argv[1],argv[2]); return -1;
    }

    struct st_girl   // 超女结构体。
    {
        int bh;
        char name[31];
    }stgirl;

    string recvbuf;

    for (int ii=0;ii<10;ii++)
    {
        memset(&stgirl,0,sizeof(stgirl));
        stgirl.bh=ii;
        sprintf(stgirl.name,"西施%d",ii);

        if (tcpclient.write(&stgirl,sizeof(stgirl))==false)        // 向服务端发送请求数据。
        {
            printf("tcpclient.write() failed.\n"); break;
        }
        cout << sformat("发送：bh=%d,name=%s\n",stgirl.bh,stgirl.name);

        sleep(1);

        if (tcpclient.read(recvbuf)==false)         // 接收服务端的回应报文。
        {
            printf("tcpclient.read() failed.\n"); break;
        }
        cout << "接收：" << recvbuf << endl;
    }
}