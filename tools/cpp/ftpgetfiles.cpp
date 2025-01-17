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
    char matchname[256];    // 待下载文件匹配的规则
    char listfilename[256]; // 调用 ftp.client.nlist() 方法列出服务器目录中的文件名，保存在本地文件 listfilename 中
    int ptype;              // 下载后服务端文件的处理方式：1-什么也不做，2-删除，3-备份
    char remotepathbak[256];// 下载后服务端文件的备份目录
    char okfilename[256];   // 已下载成功文件名
    bool checkmtime;        // 是否需要检查 ftp 服务端文件的时间，true-需要，false-不需要，缺省为 false
    int timeout;            // 进程心跳超时的时间
    char pname[51];         // 进程名，建议用 "ftpgetfiles_后缀" 的方式
}StArg;

// 文件信息的结构体
typedef struct StFileInfo {
    string filename;    // 文件名
    string mtime;       // 文件时间
    StFileInfo() = default;
    StFileInfo(const string& in_filename, const string& in_mtime) :filename(in_filename), mtime(in_mtime) {}
    void clear() { this->filename.clear();this->mtime.clear(); }
}StFileInfo;

clogfile logfile;   // 日志文件对象
cftpclient ftp;     // 创建 ftp 客户端对象
StArg st_arg;       // 程序运行的参数
Cpactive pactive;   // 进程心跳

/*
    增量下载功能：通过比较容器一和容器二中的文件信息
    - 容器三：存放本次不需要下载的文件信息
    - 容器四：存放本次需要下载的文件信息
*/
map<string, string> m_from_ok;      // 容器一：存放已下载成功的文件，从 okfilename 参数指定的文件目录中加载
list<StFileInfo> v_from_nlist;      // 容器二：下载前列出 ftp 服务器端工作目录的指定文件，从 nlist 文件中加载
list<StFileInfo> v_no_download;     // 容器三：存放本次不需要下载的文件信息
list<StFileInfo> v_download;        // 容器四：存放本次需要下载的文件信息

void EXIT(int sig);                             // 程序退出和信号 2、15 的处理函数

void help();                                    // 显示程序运行帮助文档

bool parseXML(const char* str_xml_buffer);      // 把 xml 解析到参数 st_arg 中
bool loadOKFile();                              // 加载 okfilename 文件中的内容到容器 m_from_ok 中
bool loadListFile();                            // 把 ftpclient.nlist() 方法获取到的 ftp 服务器工作目录文件信息加载到 v_from_nlist 中
bool getYesOrNoDownload();                      // 比较 v_from_nlist 和 m_from_ok，得到 v_no_download 和 v_download
bool writeNoDownloadFile();                     // 把容器 v_no_download 中的数据写入 okfilename 文件，覆盖之前旧 okfilename 文件
bool appendDownloadFile(StFileInfo& st_file_info);    // 把下载成功的文件记录追加到 okfilename 中

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
        logfile.write("parse xml failed.\n");
        return -1;
    }

    pactive.addProcInfo(st_arg.timeout, st_arg.pname);

    // 登录 ftp 服务器
    if (ftp.login(st_arg.host, st_arg.username, st_arg.password, st_arg.mode) == false) {
        logfile.write("ftp login (%s, %s, %s) failed.\n%s\n", st_arg.host, st_arg.username, st_arg.password, ftp.response());
        return -1;
    }

    logfile.write("ftp login (%s, %s, %s) OK.\n", st_arg.host, st_arg.username, st_arg.password);

    // 切换 ftp 服务器的工作目录
    if (ftp.chdir(st_arg.remotepath) == false) {
        logfile.write("ftp.chdir(%s) failed.\n%s\n", st_arg.remotepath, ftp.response());
        return -1;
    }

    // 调用 ftp.client.nlist() 方法列出服务器目录中的文件名，保存在本地文件中
    // 第一个参数使用 "." nlist 不列出文件目录，使用 st_arg.remotepath 会列出文件目录（导致 tcp 传输更多的数据）
    if (ftp.nlist(".", st_arg.listfilename) == false) {
        logfile.write("ftp.nlist(%s) failed.\n%s\n", st_arg.remotepath, ftp.response());
        return -1;
    }

    logfile.write("nlist(%s) OK.\n", st_arg.listfilename);

    pactive.updateAtime();  // 更新进程的心跳

    // 把 ftpclient.nlist() 方法获取到的 ftp 服务器工作目录文件信息加载到 v_from_nlist 中
    if (loadListFile() == false) {
        logfile.write("loadListFile() failed.\n%s\n", ftp.response());
        return -1;
    }

    if (st_arg.ptype == 1) {
        // 加载 okfilename 文件中的内容到容器 v_fromok 中
        loadOKFile();

        // 比较 v_from_list 和 m_from_ok，得到 v_no_download 和 v_download
        getYesOrNoDownload();

        // 把容器 v_no_download 中的数据写入 okfilename 文件，覆盖之前旧的 okfilename 文件
        writeNoDownloadFile();
    }
    else {
        v_from_nlist.swap(v_download);
    }

    pactive.updateAtime();  // 更新进程的心跳    

    // 遍历 v_download 容器，下载 ftp 服务端文件
    string str_remote_filename, str_local_filename;
    for (auto& tmp : v_download) {
        // 拼接 ftp 服务器端全路径名
        sformat(str_remote_filename, "%s/%s", st_arg.remotepath, tmp.filename.c_str());

        // 拼接 ftp 客户端全路径名
        sformat(str_local_filename, "%s/%s", st_arg.localpath, tmp.filename.c_str());

        logfile.write("get %s ...", str_remote_filename.c_str());

        // 调用 ftp.get() 方法下载文件
        if (ftp.get(str_remote_filename, str_local_filename, st_arg.checkmtime) == false) {
            logfile << "failed.\n" << ftp.response() << "\n";
            return -1;
        }

        // 下载成功
        logfile << "OK.\n";

        pactive.updateAtime();  // 更新进程的心跳

        // ptype == 1, 增量下载文件，把下载成功的文件记录追加到 okfilename 文件中
        if (st_arg.ptype == 1) {
            appendDownloadFile(tmp);
        }

        // ptype == 2, 删除服务端的文件
        if (st_arg.ptype == 2) {
            if (ftp.ftpdelete(str_remote_filename) == false) {
                logfile.write("ftp.ftpdelete(%s) failed.\n%s\n", str_remote_filename.c_str(), ftp.response());
                return -1;
            }
        }

        // ptype == 3, 将下载后的文件放入到 ftp 服务器的备份目录
        if (st_arg.ptype == 3) {
            // 生成全路径的备份文件名
            string str_remote_filename_bak = sformat("%s/%s", st_arg.remotepathbak, tmp.filename.c_str());
            if (ftp.ftprename(str_remote_filename, str_remote_filename_bak) == false) {
                // 需要注意的是，ftprename 执行时，在 ftp 服务器端，没有创建目录的权限，需要手动创建
                logfile.write("ftp.ftprename(%s, %s) failed.\n%s\n",
                    str_remote_filename.c_str(), str_remote_filename_bak.c_str(), ftp.response());
                return -1;
            }
        }
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
        "/CppIndustrialMeteorologyProject/tools/bin/ftpgetfiles /log/idc/ftpgetfiles_surfdata.log " \
        "\"<host>127.0.0.1:21</host>"\
        "<mode>1</mode>"\
        "<username>root</username>"\
        "<password>Hq17373546038</password>"\
        "<localpath>/idcdata/surfdata</localpath>"\
        "<remotepath>/tmp/idc/surfdata</remotepath>"\
        "<matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname>"\
        "<listfilename>/idcdata/ftplist/ftpgetfiles_surfdata.list</listfilename>"\
        "<ptype>1</ptype>"\
        "<remotepathbak>/tmp/idc/surfdatabak</remotepathbak>"\
        "<okfilename>/idcdata/ftplist/ftpgetfiles_surfdata.xml</okfilename>"\
        "<checkmtime>true</checkmtime>"\
        "<timeout>80</timeout>"\
        "<pname>ftpgetfiles_surfdata</pname>\"\n\n");

    /*
        - 下载文件后，删除 ftp 服务器上的文件
        - 下载文件后，把 ftp 服务器上的文件移动到备份目录
        - 增量下载文件，每次只下载新增的和修改过的文件
    */

    printf("本程序是通用的功能模块, 用于把远程 ftp 服务端的文件下载到本地目录;\n");
    printf("logfilename 是本程序运行的日志文件;\n");
    printf("xmlbuffer 为文件下载的参数, 如下:\n");
    printf("<host>127.0.0.1:21</host> 远程服务端的 IP 和 port;\n");
    printf("<mode>1</mode> 传输模式, 1-被动模式, 2-主动模式, 缺省采用被动模式;\n");
    printf("<username>root</username> 远程服务端 ftp 的用户名;\n");
    printf("<password>Hq17373546038</password> 远程服务端 ftp 的密码;\n");
    printf("<remotepath>/tmp/idc/surfdata</remotepath> 远程服务端存放文件的目录;\n");
    printf("<localpath>/idcdata/surfdata</localpath> 本地文件存放的目录;\n");
    printf("<matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname> 待下载文件匹配的规则, "\
        "不匹配的文件不会被下载, 本字段尽可能设置精确, 不建议用 * 匹配全部的文件;\n");
    printf("<listfilename>/idcdata/ftplist/ftpgetfiles_surfdata.list</listfilename> "\
        "执行 ftp 的 nlist 命令, 列出 ftp 服务器工作目录中所有的文件, 并且保存到 listfilename 文件中;\n");
    printf("<ptype>1</ptype> 文件下载成功后，远程服务端文件的处理方式: "\
        "1-什么也不做; 2-删除; 3-备份; 如果为 3, 还要指定备份的目录;\n");
    printf("<remotepathbak>/tmp/idc/surfdatabak</remotepathbak> 文件下载成功后，服务端文件的备份目录, "\
        "此参数只有当 ptype = 3 时才有效;\n");
    printf("<okfilename>/idcdata/ftplist/ftpgetfiles_surfdata.xml</okfilename> 上一次 ftp 连接已下载成功文件名清单, "\
        "此参数只有当 ptype = 1 时才有效;\n");
    printf("<checkmtime>true</checkmtime> 是否需要检查服务端文件的时间, true-需要, false-不需要, "\
        "此参数只有当 ptype = 1 时才有效, 缺省为 false;\n");
    printf("<timeout>80</timeout> 进程心跳，也就是下载文件超时时间, 单位: 秒, 视文件大小和网络带宽而定;\n");
    printf("<pname>ftpgetfiles_surfdata</pname> 进程名, 尽可能采用易懂的, 与其它进程不同的名称, 方便故障排查。\n\n");
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

    // nlist 命令列出工作目录中的所有文件信息，存放到 listfilename 文件中
    getxmlbuffer(str_xml_buffer, "listfilename", st_arg.listfilename, 256);
    if (strlen(st_arg.listfilename) == 0) {
        logfile.write("listfilename is null.\n");
        return false;
    }

    // 下载后服务端文件的处理方式：1-什么也不做，2-删除，3-备份
    getxmlbuffer(str_xml_buffer, "ptype", st_arg.ptype);
    if ((st_arg.ptype != 1) && (st_arg.ptype != 2) && (st_arg.ptype != 3)) {
        logfile.write("ptype is error.\n");
        return false;
    }

    // 下载后服务端文件的备份目录
    if (st_arg.ptype == 3) {
        getxmlbuffer(str_xml_buffer, "remotepathbak", st_arg.remotepathbak, 255);
        if (strlen(st_arg.remotepathbak) == 0) {
            logfile.write("remotepathbak is null.\n");
            return false;
        }
    }

    // 增量下载文件
    if (st_arg.ptype == 1) {
        // 上一次 ftp 连接已下载成功文件名清单
        getxmlbuffer(str_xml_buffer, "okfilename", st_arg.okfilename, 255);
        if (strlen(st_arg.okfilename) == 0) {
            logfile.write("okfilename is null.\n");
            return false;
        }

        // 是否需要检查服务端文件的时间，true-需要，false-不需要，此参数只有当 ptype = 1 时才有效，缺省为 false
        getxmlbuffer(str_xml_buffer, "checkmtime", st_arg.checkmtime);
    }

    // 进程心跳的超时时间
    getxmlbuffer(str_xml_buffer, "timeout", st_arg.timeout);
    if (st_arg.timeout == 0) {
        logfile.write("timeout is null.\n");
        return false;
    }

    // 进程名，缺省也没关系，所以不用判断是否为空
    getxmlbuffer(str_xml_buffer, "pname", st_arg.pname, 50);
    // if (strlen(st_arg.pname) == 0) {
    //     logfile.write("pname is null.\n");
    //     return false;
    // }

    return true;
}

// 加载 okfilename 文件中的内容到容器 m_from_ok 中
bool loadOKFile() {

    m_from_ok.clear();

    cifile ifile;

    // 注意：如果程序是第一次运行，okfilename 是不存在的，并不是错误，所以也返回 true
    if (ifile.open(st_arg.okfilename) == false) {
        return true;
    }

    string str_buffer;
    StFileInfo st_file_info;

    while (true) {
        st_file_info.clear();
        if (ifile.readline(str_buffer) == false) {
            break;
        }

        getxmlbuffer(str_buffer, "filename", st_file_info.filename);
        getxmlbuffer(str_buffer, "mtime", st_file_info.mtime);

        m_from_ok[st_file_info.filename] = st_file_info.mtime;
    }

    // for (auto& tmp : m_from_ok) {
    //     logfile.write("filename = %s, mtime = %s.\n", tmp.first.c_str(), tmp.second.c_str());
    // }

    return true;
}

// 把 ftpclient.nlist() 方法获取到的 ftp 服务器工作目录文件信息加载到 v_from_nlist 中
bool loadListFile() {
    v_from_nlist.clear();

    cifile ifile;

    if (ifile.open(st_arg.listfilename) == false) {
        logfile.write("ifile.open(%s) failed.\n", st_arg.listfilename);
        return false;
    }

    string str_filename;

    while (true) {
        // 读取一行数据存储到 str_filename
        if (ifile.readline(str_filename) == false) {
            break;
        }

        // 判断文件名是否匹配
        if (matchstr(str_filename, st_arg.matchname) == false) {
            continue;
        }

        if ((st_arg.ptype == 1) && (st_arg.checkmtime == true)) {
            // 获取 ftp 服务器端文件的最后一次修改时间
            if (ftp.mtime(str_filename) == false) {
                logfile.write("ftp.mtime(%s) failed.\n%s\n", str_filename.c_str(), ftp.response());
                return false;
            }
        }
        // c++11 新标准，允许在容器分配的存储空间中原地构造对象，然后加入到容器尾部
        v_from_nlist.emplace_back(StFileInfo(str_filename, ftp.m_mtime));
    }

    ifile.closeandremove();

    // for (auto&& tmp : v_from_nlist) {
    //     logfile.write("filename = %s, mtime = %s\n", tmp.filename.c_str(), tmp.mtime.c_str());
    // }

    return true;
}

// 比较 v_from_nlist 和 m_from_ok，得到 v_no_download 和 v_download
bool getYesOrNoDownload() {
    v_no_download.clear();
    v_download.clear();

    // 遍历 v_from_list
    for (auto& tmp : v_from_nlist) {
        auto it = m_from_ok.find(tmp.filename);     // 在容器一中用文件名查找

        if (it != m_from_ok.end()) {
            // 如果找到了，再判断文件时间
            if (st_arg.checkmtime == true) {
                // 如果时间相同，不需要下载，否则需要重新下载
                if (it->second == tmp.mtime) {
                    v_no_download.push_back(tmp);
                }
                else {
                    v_download.push_back(tmp);
                }
            }
            else {
                v_no_download.push_back(tmp);   // 不需要重新下载
            }
        }
        else {
            // 如果没有找到，把记录放入 v_download 容器
            v_download.push_back(tmp);
        }
    }
    return true;
}

// 把容器 v_no_download 中的数据写入 okfilename 文件，覆盖之前旧 okfilename 文件
bool writeNoDownloadFile() {
    cofile ofile;

    // 默认覆盖写
    if (ofile.open(st_arg.okfilename) == false) {
        logfile.write("file.open(%s) failed.\n", st_arg.okfilename);
        return false;
    }

    for (auto& tmp : v_no_download) {
        ofile.writeline("<filename>%s</filename><mtime>%s</mtime>\n", tmp.filename.c_str(), tmp.mtime.c_str());
    }

    ofile.closeandrename();
    return true;
}

// 把下载成功的文件记录追加到 okfilename 中
bool appendDownloadFile(StFileInfo& st_file_info) {
    cofile ofile;

    // 以追加的方式打开文件，注意第二个参数一定要填 false
    if (ofile.open(st_arg.okfilename, false, ios::app) == false) {
        logfile.write("file.open(%s) failed.\n", st_arg.okfilename);
        return false;
    }

    ofile.writeline("<filename>%s</filename><mtime>%s</mtime>\n", st_file_info.filename.c_str(), st_file_info.mtime.c_str());

    return true;
}