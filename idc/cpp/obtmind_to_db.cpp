#include "idcapp.h"
using namespace idc;

clogfile logfile;       // 日志文件
connection conn;        // 数据库连接
Cpactive pactive;       // 进程心跳


bool obtMindToDB(const char* pathname, const char* connstr, const char* charset);    // 业务处理主函数
void EXIT(int sig);     // 程序退出的信号处理函数


int main(int argc, char* argv[]) {

    // 帮助文档
    if (argc != 5) {
        printf("\n");
        printf("Using: ./obtmind_to_db pathname connstr charset logfile\n");

        printf("Example: /CppIndustrialMeteorologyProject/tools/bin/procctl 10 "\
            "/CppIndustrialMeteorologyProject/idc/bin/obtmind_to_db "\
            "/idcdata/surfdata "\
            "\"idc/idcpwd\" "\
            "\"Simplified Chinese_China.AL32UTF8\" "\
            "/log/idc/obtmind_to_db.log\n\n");

        printf("本程序用于把全国气象观测数据文件入库到 T_ZHOBTMIND 表中, 支持 xml 和 csv 两种文件格式, 数据只插入, 不更新;\n");
        printf("pathname: 全国气象观测数据文件存放的目录;\n");
        printf("connstr: 数据库连接参数: username/password@tnsname;\n");
        printf("charset: 数据库的字符集;\n");
        printf("logfile: 本程序运行的日志文件名;\n");
        printf("程序每 10 秒运行一次, 由procctl调度。\n\n");

        return -1;
    }

    // 1. 处理程序退出的信号，打开日志文件
    // 关闭全部的信号和输入输出
    // 设置信号，在  shell 状态下可用 "kill + 进程号" 正常终止这些进程
    // 不要用 "kill -9 + 进程号" 强行终止
    //closeioandsignal(true);
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    if (logfile.open(argv[4]) == false) {
        printf("open logfile ( %s ) failed.\n", argv[4]);
        return -1;
    }

    // 进程心跳
    pactive.addProcInfo(30, "obtmind_to_db");
    // 业务处理主函数
    obtMindToDB(argv[1], argv[2], argv[3]);

    return 0;
}

// 程序退出函数
void EXIT(int sig) {
    logfile.write("程序退出, sig = %d.\n\n", sig);

    // 可以不写，conn 的析构函数中会回滚事务与断开数据库连接
    conn.rollback();
    conn.disconnect();
    exit(0);
}

// 业务处理主函数
bool obtMindToDB(const char* pathname, const char* connstr, const char* charset) {
    // 1. 打开存放气象观测数据文件的目录
    cdir dir;
    if (dir.opendir(pathname, "*.xml,*.csv") == false) {
        logfile.write("dir.opendir(%s) failed.\n", pathname);
        return false;
    }

    CZHOBTMIND c_zhobtmind(conn, logfile);      // 操作气象观测数据表对象

    // 2. 用循环读取目录中的每个文件
    while (true) {
        // 读取一个气象观测数据文件（只处理 *.xml 和 *.csv）
        if (dir.readdir() == false) {
            logfile.write("all files of (%s) have benn processed.\n\n", pathname);
            break;
        }

        // 如果有文件需要处理，判断与数据库的连接状态，如果未连接，就连接上
        if (conn.isopen() == false) {
            if (conn.connecttodb(connstr, charset) != 0) {
                logfile.write("connect database(%s) failed.\n%s\n", connstr, conn.message());
                return false;
            }
            logfile.write("connect database(%s) ok.\n", connstr);
        }

        // 打开文件，读取文件中的每一行，插入到数据库的表中
        cifile ifile;
        if (ifile.open(dir.m_ffilename) == false) {
            logfile.write("file.open(%s) failed.\n", dir.m_ffilename.c_str());
            return false;
        }

        int total_count = 0;    // 文件的总记录数
        int insert_count = 0;   // 成功插入记录数
        ctimer timer;           // 计时器，记录每个数据文件的处理耗时
        bool is_xml = matchstr(dir.m_ffilename, "*.xml");    // 文件格式，true-xml, false-csv

        string str_buffer;      // 存放从文件中读取的一行数据
        if (is_xml == false) {
            ifile.readline(str_buffer);
        }

        while (true) {
            // 从文件中读取一行
            if (is_xml == true) {
                // xml 文件，<endl/> 是行结束标志
                if (ifile.readline(str_buffer, "<endl/>") == false) {
                    break;
                }
            }
            else {
                // csv 文件，没有结束标志
                if (ifile.readline(str_buffer) == false) {
                    break;
                }
            }
            ++total_count;

            // 解析行的内容（*.xml 和 *.csv 的方法不同），把数据存放到结构体中
            c_zhobtmind.splitBuffer(str_buffer, is_xml);

            // 把解析后的数据插入到数据库表中
            if (c_zhobtmind.insertTable() == true) {
                ++insert_count;     // 成功插入的记录数加 1
            }
        }

        // 关闭并且删除已处理的文件，提交事务
        ifile.closeandremove();
        conn.commit();
        logfile.write("Have proceeded file %s(total_count = %d, insert_count = %d), spend time %.2f s.\n",
            dir.m_ffilename.c_str(), total_count, insert_count);
    }

    return true;
}