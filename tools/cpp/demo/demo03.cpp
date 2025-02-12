#include "_public.h"
using namespace idc;

ctcpclient tcp_client;   // TCP 通信的客户端

string str_send_buffer;
string str_recv_buffer;

bool biz001();      // 登录
bool biz002();      // 查询余额
bool biz003();      // 转账

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Using: ./demo03 ip port\n");
        printf("Example: ./demo03 127.0.0.1 5005\n\n");
        return -1;
    }
    if (tcp_client.connect(argv[1], atoi(argv[2])) == false) {
        printf("tcp_client.connect() failed.\n");
        return -1;
    }

    biz001();   // 登录

    biz002();   // 查询余额

    biz003();   // 转账

    return 0;
}

// 登录
bool biz001() {
    str_send_buffer = "<bizid>1</bizid>"\
        "<username>13922200001</username>"\
        "<password>123456</password>";
    if (tcp_client.write(str_send_buffer) == false) {
        printf("tcp_client.write() failed.\n");
        return false;
    }

    cout << "发送：" << str_send_buffer << '\n';

    if (tcp_client.read(str_recv_buffer) == false) {
        printf("tcp_client.read() failed.\n");
        return false;
    }

    cout << "接收：" << str_recv_buffer << '\n';

    return true;
}

// 查询余额
bool biz002() {
    str_send_buffer = "<bizid>2</bizid>"\
        "<cardid>6262000000001</cardid>";
    if (tcp_client.write(str_send_buffer) == false) {
        printf("tcp_client.write() failed.\n"); return false;
    }

    cout << "发送：" << str_send_buffer << endl;

    if (tcp_client.read(str_recv_buffer) == false) {
        printf("tcp_client.read() failed.\n"); return false;
    }

    cout << "接收：" << str_recv_buffer << endl;

    return true;
}

// 转账
bool biz003() {
    str_send_buffer = "<bizid>3</bizid>"\
        "<cardid1>6262000000001</cardid1>"\
        "<cardid2>6262000000001</cardid2>"\
        "<je>100.8</je>";
    if (tcp_client.write(str_send_buffer) == false) {
        printf("tcp_client.write() failed.\n"); return false;
    }

    cout << "发送：" << str_send_buffer << endl;

    if (tcp_client.read(str_recv_buffer) == false) {
        printf("tcp_client.read() failed.\n"); return false;
    }

    cout << "接收：" << str_recv_buffer << endl;

    return true;
}

