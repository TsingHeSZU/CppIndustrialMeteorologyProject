/*
 *  程序名：demo51.cpp，此程序演示采用开发框架的 cftpclient 类上传文件。
 *  作者：吴从周
*/
#include "../_ftp.h"

using namespace idc;

int main(int argc, char* argv[])
{
    cftpclient ftp;

    // 登录远程ftp服务器
    if (ftp.login("172.30.192.45:21", "utopianyouth", "123") == false) {
        printf("ftp.login(172.30.192.45:21, utopianyouth/123) failed.\n");
        return -1;
    }

    // 在ftp服务器上创建/home/wucz/tmp，注意，如果目录已存在，会返回失败。
    if (ftp.mkdir("/home/utopianyouth/tmp") == false) {
        printf("ftp.mkdir() failed.\n");
        return -1;
    }

    // 把 ftp 服务器上的工作目录切换到 /home/utopianyouth/tmp
    if (ftp.chdir("/home/utopianyouth/tmp") == false) {
        printf("ftp.chdir() failed.\n");
        return -1;
    }

    // 把本地的 demo51.cpp 上传到 ftp 服务器的当前工作目录。
    if (ftp.put("demo51.cpp", "demo51.cpp") == true) {
        printf("put demo51.cpp ok.\n");
    }
    else {
        printf("put demo51.cpp failed.\n");
    }
    return 0;
}

