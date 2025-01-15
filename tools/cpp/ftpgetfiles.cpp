#include "/CppIndustrialMeteorologyProject/public/_public.h"
#include "/CppIndustrialMeteorologyProject/public/_ftp.h"
using namespace idc;

// 程序运行参数的结构体
typedef struct StArg
{
    char host[31];          // 远程服务端的 IP 和 port
    int mode;               // 传输模式： 1-被动模式，2-主动模式，缺省采用被动模式
    char username[31];      // 远程服务端 ftp 的用户名
    char password[31];      // 远程服务端 ftp 的密码
    char remotepath[256];   // 远程服务端存放文件的目录
    char localpath[256];    // 本地文件存放的目录
    char matchname[101];    // 待下载文件匹配的规则
}StArg;

// 存放文件信息的结构体
typedef struct StFileInfo {
    string filename;    // 文件名
    string mtime;       // 文件时间
    StFileInfo() = default;
    StFileInfo(const string& in_filename, const string& in_mtime) :filename(in_filename), mtime(in_mtime) {}
    void clear() { this->filename.clear();this->mtime.clear(); }
}StFileInfo;

// 程序退出和信号 2、15 的处理函数
void EXIT(int sig);

// 显示程序运行帮助文档
void help();

// 把 xml 解析到参数 st_arg 中
bool parseXML(const char* str_xml_buffer);

// 把 ftpclient.nlist() 方法获取到的 ftp 服务器工作目录文件信息加载到 vfilelist 中
bool loadListFile();

clogfile logfile;   // 日志文件对象
cftpclient ftp;     // 创建 ftp 客户端对象
StArg st_arg;       // 程序运行的参数
vector<StFileInfo> vfilelist;   // 存储 ftp 服务器端 nlist 命令得出来的文件信息


int main(int argc, char* argv[]) {
    /*
        设置信号, 在 shell 状态下可用 "kill + 进程号" 正常终止
        不要用 "kill -9 + 进程号" 强行终止
    */
    // closeioandsignal(true);     // 关闭 I/O 和忽略全部的信号, 在调试阶段, 不启用
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    // 1. 从服务器某个目录中下载文件, 可以指定文件名匹配的规则
    if (argc != 3) {
        help();
        return -1;
    }

    // 打开日志文件
    if (logfile.open(argv[1]) == false) {
        printf("打开日志文件 (%s) 失败\n", argv[1]);
        return -1;
    }

    //解析 xml, 得到程序运行的参数
    if (parseXML(argv[2]) == false) {
        return -1;
    }

    // 登录 ftp 服务器
    if (ftp.login(st_arg.host, st_arg.username, st_arg.password, st_arg.mode) == false) {
        logfile.write("ftp login (%s, %s, %s) failed.\n", st_arg.host, st_arg.username, st_arg.password);
        return -1;
    }

    logfile.write("ftp login (%s, %s, %s) OK.\n", st_arg.host, st_arg.username, st_arg.password);

    // 切换 ftp 服务器的工作目录
    if (ftp.chdir(st_arg.remotepath) == false) {
        logfile.write("ftp.chdir(%s) failed.\n", st_arg.remotepath);
        return -1;
    }

    // 调用 ftp.client.nlist() 方法列出服务器目录中的文件名，保存在本地文件中
    // 第一个参数使用 "." nlist 不列出文件目录，使用 st_arg.remotepath 会列出文件目录（导致 tcp 传输更多的数据）
    if (ftp.nlist(".", sformat("/tmp/nlist/ftpgetfiles_%d.nlist", getpid())) == false) {
        logfile.write("ftp.nlist(%s) failed.\n", st_arg.remotepath);
        return -1;
    }

    logfile.write("nlist(%s) OK.\n", sformat("/tmp/nlist/ftpgetfiles_%d.nlist", getpid()).c_str());

    // 把 ftpclient.nlist() 方法获取到的 ftp 服务器工作目录文件信息加载到 vfilelist 中
    if (loadListFile() == false) {
        logfile.write("loadListFile() failed.\n");
        return -1;
    }

    // 遍历 vfilelist 容器，下载 ftp 服务端文件
    string str_remote_filename, str_local_filename;
    for (auto& tmp : vfilelist) {
        // 拼接 ftp 服务器端全路径名
        sformat(str_remote_filename, "%s/%s", st_arg.remotepath, tmp.filename.c_str());

        // 拼接 ftp 客户端全路径名
        sformat(str_local_filename, "%s/%s", st_arg.localpath, tmp.filename.c_str());

        logfile.write("get %s ...", str_remote_filename.c_str());

        // 调用 ftp.get() 方法下载文件
        if (ftp.get(str_remote_filename, str_local_filename) == false) {
            logfile << "failed.\n";
            return -1;
        }

        // 下载成功
        logfile << "OK.\n";
    }

    return 0;
}

// 程序退出
void EXIT(int sig) {
    printf("程序退出, sig = %d\n", sig);
    exit(0);
}

// 显示程序运行帮助文档
void help() {
    printf("\nUsing: /CppIndustrialMeteorologyProject/tools/bin/ftpgetfiles logfilename xmlbuffer\n\n");
    printf("Example: /CppIndustrialMeteorologyProject/tools/bin/procctl 30 "\
        "/CppIndustrialMeteorologyProject/tools/bin/ftpgetfiles /log/idc/ftpgetfiles_test.log " \
        "\"<host>127.0.0.1:21</host><mode>1</mode>"\
        "<username>utopianyouth</username><password>123</password>"\
        "<remotepath>/tmp/idc/surfdata</remotepath><localpath>/tmp/ftp/client</localpath>"\
        "<matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname>\"\n\n");
    printf("本程序是通用的功能模块, 用于把远程ftp服务端的文件下载到本地目录; \n");
    printf("logfilename是本程序运行的日志文件; \n");
    printf("xmlbuffer为文件下载的参数, 如下: \n");
    printf("<host>192.168.150.128:21</host> 远程服务端的IP和端口; \n");
    printf("<mode>1</mode> 传输模式, 1-被动模式, 2-主动模式, 缺省采用被动模式; \n");
    printf("<username>utopianyouth</username> 远程服务端ftp的用户名; \n");
    printf("<password>123</password> 远程服务端ftp的密码; \n");
    printf("<remotepath>/tmp/idc/surfdata</remotepath> 远程服务端存放文件的目录; \n");
    printf("<localpath>/idcdata/surfdata</localpath> 本地文件存放的目录; \n");
    printf("<matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname> 待下载文件匹配的规则。"\
        "不匹配的文件不会被下载, 本字段尽可能设置精确, 不建议用*匹配全部的文件。\n");
}

// 把 xml 解析到参数 st_arg 中
bool parseXML(const char* str_xml_buffer) {
    memset(&st_arg, 0, sizeof(st_arg));     // char 被初始化为 '\0'

    // 远程服务端的 ip 和 port
    getxmlbuffer(str_xml_buffer, "host", st_arg.host, 30);  // 30 限制解析的长度  
    if (strlen(st_arg.host) == 0) {
        logfile.write("host is null.\n");
        return false;
    }

    // 传输模式，1-被动模式，2-主动模式，缺省采用被动模式
    getxmlbuffer(str_xml_buffer, "mode", st_arg.mode);
    if (st_arg.mode != 2) {
        st_arg.mode = 1;    // 填错或者缺省都为 1
    }

    // 远程服务端 ftp 的用户名
    getxmlbuffer(str_xml_buffer, "username", st_arg.username, 30);
    if (strlen(st_arg.username) == 0) {
        logfile.write("username is null.\n");
        return false;
    }

    // 远程服务端 ftp 的密码
    getxmlbuffer(str_xml_buffer, "password", st_arg.password, 30);
    if (strlen(st_arg.password) == 0) {
        logfile.write("password is null.\n");
        return false;
    }

    // 远程服务端存放文件的目录
    getxmlbuffer(str_xml_buffer, "remotepath", st_arg.remotepath, 255);
    if (strlen(st_arg.remotepath) == 0) {
        logfile.write("remotepath is null.\n");
        return false;
    }

    // 本地客户端存放文件的目录
    getxmlbuffer(str_xml_buffer, "localpath", st_arg.localpath, 255);
    if (strlen(st_arg.remotepath) == 0) {
        logfile.write("localpath is null.\n");
        return false;
    }

    // 待下载文件匹配的规则
    getxmlbuffer(str_xml_buffer, "matchname", st_arg.matchname, 100);
    if (strlen(st_arg.matchname) == 0) {
        logfile.write("matchname is null.\n");
        return false;
    }

    return true;
}

// 把 ftpclient.nlist() 方法获取到的 ftp 服务器工作目录文件信息加载到 vfilelist 中
bool loadListFile() {
    vfilelist.clear();

    cifile ifile;

    if (ifile.open(sformat("/tmp/nlist/ftpgetfiles_%d.nlist", getpid())) == false) {
        logfile.write("ifile.open(%s) failed.\n", sformat("/tmp/nlist/ftpgetfiles_%d.nlist", getpid()).c_str());
        return false;
    }

    string str_filename;

    while (true) {
        if (ifile.readline(str_filename) == false) {
            break;
        }

        // 判断文件名是否匹配
        if (matchstr(str_filename, st_arg.matchname) == false) {
            continue;
        }

        // c++11 新标准，允许在容器分配的存储空间中原地构造对象，然后加入到容器尾部
        vfilelist.emplace_back(StFileInfo(str_filename, ""));
    }

    ifile.closeandremove();

    // for (auto&& tmp : vfilelist) {
    //     logfile.write("filename = %s\n", tmp.filename.c_str());
    // }

    return true;
}