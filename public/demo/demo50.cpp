/*
 *  程序名：demo50.cpp，此程序演示采用开发框架的 cftpclient 类获取 ftp 服务器上的文件列表、时间和大小
 *  作者：吴从周
*/
#include "../_ftp.h"

using namespace idc;

int main(int argc, char* argv[])
{
    cftpclient ftp;

    // 登录远程ftp服务器
    if (ftp.login("172.30.192.45:21", "utopianyouth", "123") == false) {
        printf("ftp.login(172.30.192.45:21,utopianyouth/123) failed.\n");
        return -1;
    }


    // 获取服务器上/project/public/*.h文件列表，保存在本地的/tmp/list/tmp.list文件中。
    // 如果/tmp/list目录不存在，会自动创建它。
    if (ftp.nlist("/CppIndustrialMeteorologyProject/public/*.h", "/tmp/list/tmp.list") == false) {
        printf("ftp.nlist() failed.\n");
        return -1;
    }
    cout << "ret = " << ftp.response() << endl;

    cifile ifile;    // 采用开发框架的cifile类来操作list文件。
    string strFileName;

    ifile.open("/tmp/list/tmp.list");  // 打开list文件。

    while (true) {
        // 获取每个文件的时间和大小
        if (ifile.readline(strFileName) == false) {
            break;
        }

        ftp.mtime(strFileName); // 获取文件时间
        ftp.size(strFileName);  // 获取文件大小

        printf("filename = %s,mtime = %s,size = %d\n", strFileName.c_str(), ftp.m_mtime.c_str(), ftp.m_size);
    }
}