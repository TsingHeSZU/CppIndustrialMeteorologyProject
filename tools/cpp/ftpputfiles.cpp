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
    char matchname[256];    // 待上传文件匹配的规则
    int ptype;              // 上传后客户端文件的处理方式：1-什么也不做，2-删除，3-备份
    char localpathbak[256]; // 上传后客户端文件的备份目录
    char okfilename[256];   // 该文件存放了上一次已上传成功的文件列表
    int timeout;            // 进程心跳超时的时间
    char pname[51];         // 进程名，建议用 "ftpputfiles_后缀" 的方式
}StArg;

// 文件信息的结构体
typedef struct StFileInfo {
    string filename;    // 文件名
    string mtime;       // 文件时间
    StFileInfo() = default;     // 启用默认构造函数
    StFileInfo(const string& in_filename, const string& in_mtime) :filename(in_filename), mtime(in_mtime) {}
    void clear() { this->filename.clear();this->mtime.clear(); }
}StFileInfo;

clogfile logfile;   // 日志文件对象
cftpclient ftp;     // ftp 客户端对象
StArg st_arg;       // 程序运行的参数
Cpactive pactive;   // 进程心跳对象

/*
    增量上传功能：通过比较容器一和容器二中的文件信息
    - 容器三：存放本次不需要上传的文件信息
    - 容器四：存放本次需要上传的文件信息
*/
map<string, string> m_from_ok;      // 容器一：存放已上传成功的文件，从 okfilename 参数指定的文件目录中加载
list<StFileInfo> v_from_dir;        // 容器二：客户端目录中的文件名
list<StFileInfo> v_no_upload;       // 容器三：本次不需要上传的文件
list<StFileInfo> v_upload;          // 容器四：本次需要上传的文件

void EXIT(int sig);                             // 程序退出和信号 2、15 的处理函数

void help();                                    // 显示程序运行帮助文档

bool parseXML(const char* str_xml_buffer);      // 把 xml 解析到参数 st_arg 中
bool loadOKFile();                              // 加载 okfilename 文件中的内容到容器 m_from_ok 中
bool loadLocalFile();                           // 把 localpath 目录下的文件列表加载到 v_from_dir 容器中
bool getYesOrNoUpload();                        // 比较 v_from_dir 和 m_from_ok，得到 v_no_upload 和 v_upload
bool writeNoUploadFile();                       // 把容器 v_no_upload 中的数据写入 okfilename 文件，覆盖之前旧 okfilename 文件
bool appendUploadFile(StFileInfo& st_file_info);    // 把上传成功的文件记录追加到 okfilename 中

int main(int argc, char* argv[]) {
    /*
        设置信号, 在 shell 状态下可用 "kill + 进程号" 正常终止
        不要用 "kill -9 + 进程号" 强行终止
    */
    // closeioandsignal(true);     // 关闭 I/O 和忽略全部的信号, 在调试阶段, 不启用
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    // 1. 上传本地文件到 ftp 服务器中
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

    // 把 localpath 目录下的文件列表加载到 v_from_dir 容器中
    if (loadLocalFile() == false) {
        logfile.write("loadLocalFile() failed.\n");
        return -1;
    }

    pactive.updateAtime();  // 更新进程的心跳


    if (st_arg.ptype == 1) {
        // 加载 okfilename 文件中的内容到容器 v_fromok 中
        loadOKFile();

        // 比较 v_from_dir 和 m_from_ok，得到 v_no_uplaod 和 v_upload
        getYesOrNoUpload();

        // 把容器 v_no_upload 中的数据写入 okfilename 文件，覆盖之前旧的 okfilename 文件
        writeNoUploadFile();
    }
    else {
        v_from_dir.swap(v_upload);
    }

    pactive.updateAtime();  // 更新进程的心跳    

    // 遍历 v_upload 容器，上传客户端的文件到 ftp 服务器中
    string str_remote_filename, str_local_filename;
    for (auto& tmp : v_upload) {
        // 拼接 ftp 服务器端全路径名
        sformat(str_remote_filename, "%s/%s", st_arg.remotepath, tmp.filename.c_str());

        // 拼接 ftp 客户端全路径名
        sformat(str_local_filename, "%s/%s", st_arg.localpath, tmp.filename.c_str());

        logfile.write("put %s ...", str_local_filename.c_str());

        // 调用 ftp.put() 方法把文件上传到 ftp 服务器，第三个参数填 true 的目的是确保文件上传成功，对方不可抵赖

        if (ftp.put(str_local_filename, str_remote_filename, true) == false) {
            logfile << "failed.\n" << ftp.response() << "\n";
            printf("local = %s, remote = %s\n", str_local_filename.c_str(), str_remote_filename.c_str());
            return -1;
        }

        // 上传成功
        logfile << "OK.\n";

        pactive.updateAtime();  // 更新进程的心跳

        // ptype == 1, 增量上传文件，把上传成功的文件记录追加到 okfilename 文件中
        if (st_arg.ptype == 1) {
            appendUploadFile(tmp);
        }

        // ptype == 2, 删除客户端上传后的文件
        if (st_arg.ptype == 2) {
            if (remove(str_local_filename.c_str()) == false) {
                logfile.write("remove(%s) failed.\n", str_local_filename.c_str());
                return -1;
            }
        }

        // ptype == 3, 将上传后的文件放入到 ftp 客户端的备份目录
        if (st_arg.ptype == 3) {
            // 生成全路径的备份文件名
            string str_local_filename_bak = sformat("%s/%s", st_arg.localpathbak, tmp.filename.c_str());
            if (renamefile(str_local_filename, str_local_filename_bak) == false) {
                // 本地备份失败
                logfile.write("renamefile(%s, %s) failed.\n",
                    str_local_filename.c_str(), str_local_filename_bak.c_str());
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
    printf("\n");
    printf("Using: /CppIndustrialMeteorologyProject/tools/bin/ftpputfiles logfilename xmlbuffer\n\n");

    printf("Example: /CppIndustrialMeteorologyProject/tools/bin/procctl 30 "\
        "/CppIndustrialMeteorologyProject/tools/bin/ftpputfiles /log/idc/ftpputfiles_surfdata.log " \
        "\"<host>127.0.0.1:21</host>"\
        "<mode>1</mode>"\
        "<username>root</username>"\
        "<password>Hq17373546038</password>"\
        "<localpath>/tmp/idc/surfdata</localpath>"\
        "<remotepath>/tmp/ftpputtest</remotepath>"\
        "<matchname>SURF_ZH*.JSON</matchname>"\
        "<ptype>1</ptype>"\
        "<localpathbak>/tmp/idc/surfdatabak</localpathbak>"\
        "<okfilename>/idcdata/ftplist/ftpputfiles_surfdata.xml</okfilename>"\
        "<timeout>80</timeout>"\
        "<pname>ftpputfiles_surfdata</pname>\"\n\n");

    /*
        - 上传文件后，删除本地客户端上的文件
        - 上传文件后，把本地客户端上的文件移动到备份目录
        - 增量上床文件，每次只上传新增的和修改过的文件
    */

    printf("本程序是通用的功能模块, 用于把本地目录中的文件上传到远程的ftp服务器;\n");
    printf("logfilename 是本程序运行的日志文件;\n");
    printf("xmlbuffer 为文件上传的参数, 如下:\n");
    printf("<host>127.0.0.1:21</host> 远程服务端的 IP 和 port;\n");
    printf("<mode>1</mode> 传输模式, 1-被动模式, 2-主动模式, 缺省采用被动模式;\n");
    printf("<username>root</username> 远程服务端 ftp 的用户名;\n");
    printf("<password>Hq17373546038</password> 远程服务端 ftp 的密码;\n");
    printf("<localpath>/tmp/idc/surfdata</localpath> 本地文件存放的目录;\n");
    printf("<remotepath>/tmp/ftpputtest</remotepath> 远程服务端存放文件的目录;\n");
    printf("<matchname>SURF_ZH*.JSON</matchname> 待下载文件匹配的规则;"\
        "不匹配的文件不会被下载, 本字段尽可能设置精确, 不建议用 * 匹配全部的文件;\n");
    printf("<ptype>1</ptype> 文件上传成功后, 本地文件的处理方式: "\
        "1-什么也不做; 2-删除; 3-备份; 如果为 3, 还要指定备份的目录;\n");
    printf("<localpathbak>/tmp/idc/surfdatabak</localpathbak> 文件上传成功后, 服务端文件的备份目录, "\
        "此参数只有当 ptype = 3 时才有效;\n");
    printf("<okfilename>/idcdata/ftplist/ftpgetfiles_test.xml</okfilename> 上一次 ftp 连接已上传成功文件名清单, "\
        "此参数只有当 ptype = 1 时才有效;\n");
    printf("<timeout>80</timeout> 进程心跳, 也就是上传文件超时时间, 单位: 秒, 视文件大小和网络带宽而定;\n");
    printf("<pname>ftpputfiles_test</pname> 进程名, 尽可能采用易懂的, 与其它进程不同的名称, 方便故障排查。\n\n");
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

    // 本地客户端存放文件的目录
    getxmlbuffer(str_xml_buffer, "localpath", st_arg.localpath, 255);
    if (strlen(st_arg.localpath) == 0) {
        logfile.write("localpath is null.\n");
        return false;
    }

    // 远程服务端存放文件的目录
    getxmlbuffer(str_xml_buffer, "remotepath", st_arg.remotepath, 255);
    if (strlen(st_arg.remotepath) == 0) {
        logfile.write("remotepath is null.\n");
        return false;
    }

    // 待上传文件匹配的规则
    getxmlbuffer(str_xml_buffer, "matchname", st_arg.matchname, 100);
    if (strlen(st_arg.matchname) == 0) {
        logfile.write("matchname is null.\n");
        return false;
    }

    // 上传后服务端文件的处理方式：1-什么也不做，2-删除，3-备份
    getxmlbuffer(str_xml_buffer, "ptype", st_arg.ptype);
    if ((st_arg.ptype != 1) && (st_arg.ptype != 2) && (st_arg.ptype != 3)) {
        logfile.write("ptype is error.\n");
        return false;
    }

    // 上传后客户端文件的备份目录
    if (st_arg.ptype == 3) {
        getxmlbuffer(str_xml_buffer, "localpathbak", st_arg.localpathbak, 255);
        if (strlen(st_arg.localpathbak) == 0) {
            logfile.write("localpathbak is null.\n");
            return false;
        }
    }

    // 增量上传文件
    if (st_arg.ptype == 1) {
        // 上一次 ftp 连接已上传成功文件名清单
        getxmlbuffer(str_xml_buffer, "okfilename", st_arg.okfilename, 255);
        if (strlen(st_arg.okfilename) == 0) {
            logfile.write("okfilename is null.\n");
            return false;
        }
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

// 把 localpath 目录下的文件列表加载到 v_from_dir 容器中
bool loadLocalFile() {
    v_from_dir.clear();

    cdir dir;

    // 不包括子目录
    if (dir.opendir(st_arg.localpath, st_arg.matchname) == false) {
        logfile.write("dir.opendir(%s) failed.\n", st_arg.localpath);
        return false;
    }

    while (true) {
        if (dir.readdir() == false) {
            break;
        }

        v_from_dir.emplace_back(StFileInfo(dir.m_filename, dir.m_mtime));
    }

    // for (auto&& tmp : v_from_dir) {
    //     logfile.write("filename = %s, mtime = %s\n", tmp.filename.c_str(), tmp.mtime.c_str());
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

// 比较 v_from_nlist 和 m_from_ok，得到 v_no_download 和 v_download
bool getYesOrNoUpload() {
    v_no_upload.clear();
    v_upload.clear();

    // 遍历 v_from_dir
    for (auto& tmp : v_from_dir) {
        auto it = m_from_ok.find(tmp.filename);     // 在容器一中用文件名查找

        if (it != m_from_ok.end()) {
            // 找到了且时间相同，不需要上传，否则需要重新下载
            if (it->second == tmp.mtime) {
                v_no_upload.push_back(tmp);
            }
            else {
                v_upload.push_back(tmp);
            }
        }
        else {
            // 没有找到，ftp 服务器中不存在该文件，需要上传
            v_upload.push_back(tmp);
        }
    }
    return true;
}

// 把容器 v_no_upload 中的数据写入 okfilename 文件，覆盖之前旧 okfilename 文件
bool writeNoUploadFile() {
    cofile ofile;

    // 默认覆盖写
    if (ofile.open(st_arg.okfilename) == false) {
        logfile.write("file.open(%s) failed.\n", st_arg.okfilename);
        return false;
    }

    for (auto& tmp : v_no_upload) {
        ofile.writeline("<filename>%s</filename><mtime>%s</mtime>\n", tmp.filename.c_str(), tmp.mtime.c_str());
    }

    ofile.closeandrename();
    return true;
}

// 把下载成功的文件记录追加到 okfilename 中
bool appendUploadFile(StFileInfo& st_file_info) {
    cofile ofile;

    // 以追加的方式打开文件，注意第二个参数一定要填 false
    if (ofile.open(st_arg.okfilename, false, ios::app) == false) {
        logfile.write("file.open(%s) failed.\n", st_arg.okfilename);
        return false;
    }
    ofile.writeline("<filename>%s</filename><mtime>%s</mtime>\n", st_file_info.filename.c_str(), st_file_info.mtime.c_str());

    return true;
}