/*
 *  程序名：demo52.cpp，此程序演示采用开发框架的 cftpclient 类从 ftp 服务器下载文件。
 *  作者：吴从周
*/
#include "../_ftp.h"

using namespace idc;

int main(int argc, char* argv[])
{
    cftpclient ftp;

    // 登录远程ftp服务器，请改为你自己服务器的ip地址。
    if (ftp.login("127.0.0.1:21", "utopianyouth", "123") == false) {
        printf("ftp.login(127.0.0.1:21, utopianyouth/123) failed.\n");
        return -1;
    }

    // 把服务器上的 /home/utopianyouth/tmp/demo51.cpp 下载到本地，存为 /tmp/test/demo51.cpp
    // 如果本地的 /tmp/test 目录不存在，get 函数会创建它
    if (ftp.get("/home/utopianyouth/tmp/demo51.cpp", "/tmp/test/demo51.cpp") == false) {
        printf("ftp.get() failed.\n"); return -1;
    }

    printf("get /home/utopianyouth/tmp/demo51.cpp ok.\n");

    // 删除服务上的/home/utopianyouth/tmp/demo51.cpp文件
    if (ftp.ftpdelete("/home/utopianyouth/tmp/demo51.cpp") == false) {
        printf("ftp.ftpdelete() failed.\n");
        return -1;
    }

    printf("delete /home/utopianyouth/tmp/demo51.cpp ok.\n");

    // 删除服务器上的 /home/utopianyouth/tmp 目录，如果目录非空，删除将失败
    if (ftp.rmdir("/home/utopianyouth/tmp") == false) {
        printf("ftp.rmdir() failed.\n");
        return -1;
    }
}

