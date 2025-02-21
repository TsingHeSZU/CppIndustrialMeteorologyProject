/*
    程序功能：把数据抽取模块生成的 xml 文件入库到 oracle 的表中
*/
#include "_tools.h"
using namespace idc;

// 程序运行参数的结构体
typedef struct st_arg {
    char connstr[101];              // 数据库的连接参数
    char charset[51];               // 数据库的字符集
    char inifilename[301];          // 数据入库的参数配置文件
    char xmlpath[301];              // 待入库 xml 文件存放的目录
    char xmlpathbak[301];           // xml 文件入库后的备份目录
    char xmlpatherr[301];           // 入库失败的 xml 文件存放的目录
    int timetvl;                    // 本程序运行的时间间隔，本程序常驻内存
    int timeout;                    // 本程序运行时的超时时间
    char pname[51];                 // 本程序运行时的程序名
} StArg;

// 数据入库参数的结构体
typedef struct StXMLToTable {
    char filename[101];     // xml 文件的匹配规则，用逗号分隔
    char tname[31];         // 待入库的表名
    int updateflag;         // 入库时有重复记录的更新标志，1-更新，2-不更新
    char execsql[301];      // 处理 xml 文件之前，执行的 sql 语句，比如数据入库之前，需要删除对应表中内容
}StXMLToTable;

Cpactive pactive;       // 进程心跳
StArg starg;            // 程序运行参数对象
clogfile logfile;       // 本程序运行的日志
connection conn;        // 数据库连接
StXMLToTable stxmltotable;  // 数据入库参数结构体

ctcols tcols;           // 获取表全部的字段和主键字段
string str_insertsql;   // 插入表的 sql 语句
string str_updatesql;   // 更新表的 sql 语句
sqlstatement stmt_insert;   // 执行插入 sql 语句对象
sqlstatement stmt_update;   // 执行更新 sql 语句对象
ctimer timer;           // 处理每个 xml 文件消耗的时间
int totalcount;         // 每个 xml 文件的总记录数
int insertcount;        // 每个 xml 文件插入数据库表的记录数
int updatecount;        // 每个 xml 文件更新数据库表的记录数

vector<StXMLToTable> v_xmltotable;   // 数据入库参数的容器
vector<string> v_colvalue;  // 存放从 xml 每一行中解析出来的字段的值，将用于插入和更新表的 SQL 语句绑定变量

// 程序帮助文档
void Help(char* argv[]);
// 把 xml 解析到参数 starg 结构体中
bool parseXMLToArg(const char* strxmlbuffer);
// 业务处理主函数
bool xmlToDatabaseMain();
// 把数据入库的参数配置文件 starg.inifilename 加载到 v_xmltotable 容器中
bool loadXMLToVector();
// 根据文件名，从 v_xmltotable 容器中查找的入库参数，存放在 StXMLToTable 结构体中
bool findOpeDBParameterFromVector(const string& xmlfilename);

/*
    处理 xml 文件的子函数
    返回值：
    - 0：成功
    - 1：入库参数配置不正确（无法找到 xml 文件需要入库的数据库表）
    - 2：数据库系统有问题，或网络断开，或连接超时（无法得到 xml 文件对应数据库表的字段信息和主键）
    - 3：xml 入库对应的数据库表不存在（即程序中，存储数据库表全部字段的容器大小为 0，也有可能入库参数配置错误）
    - 4：执行删除数据库内容的 sql 语句失败，即入库参数配置文件的 <execsql></execsql> 项内容，如果有这项，会在 xml 入库前执行该 sql 语句
    - 5：打开 xml 文件失败，无法执行入库操作
*/
int xmlToDatabase(const string& fullfilename, const string& filename);

// 拼接 sql 语句
void jointSQL();

// 动态 sql 绑定变量
void prepareSQL();

// 在打开 xml 文件之前，如果 stxmltotable.execsql 不为空，就执行它
bool execsql();

// 解析 xml 文件中读取的每一行数据
void splitXMLBuffer(const string& strbuffer);

// 把 xml 文件移动到备份目录或错误目录
bool xmlToBakErr(const string& fullfilename, const string& srcpath, const string& dstpath);

// 程序退出的信号处理函数
void EXIT(int sig);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        Help(argv);
        return -1;
    }

    // 关闭全部的信号和输入输出
    // shell 状态下可用 "kill + 进程号" 正常终止进程
    // 不要用 "kill -9 + 进程号" 强制终止进程
    //closeioandsignal(true);
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    if (logfile.open(argv[1]) == false) {
        printf("open logfile(%s) failed.\n", argv[1]);
        return -1;
    }

    // 把 xml 解析到程序运行参数 starg 结构体中
    if (parseXMLToArg(argv[2]) == false) {
        logfile.write("parse xml(%s) for program running failed.\n", argv[2]);
        return -1;
    }

    // 添加进程心跳信息到共享内存中
    pactive.addProcInfo(starg.timeout, starg.pname);

    // 业务处理主函数
    xmlToDatabaseMain();

    return 0;
}

// 程序帮助文档
void Help(char* argv[]) {
    printf("Using: /CppIndustrialMeteorologyProject/tools/bin/xmltodb logfilename xmlbuffer\n");

    printf("Sample:\n/CppIndustrialMeteorologyProject/tools/bin/procctl 10 "\
        "/CppIndustrialMeteorologyProject/tools/bin/xmltodb /log/idc/xmltodb_vip.log "\
        "\"<connstr>idc/idcpwd</connstr>"\
        "<charset>Simplified Chinese_China.AL32UTF8</charset>"\
        "<inifilename>/CppIndustrialMeteorologyProject/idc/ini/xmltodb.xml</inifilename>"\
        "<xmlpath>/idcdata/xmltodb/vip</xmlpath>"\
        "<xmlpathbak>/idcdata/xmltodb/vipbak</xmlpathbak>"\
        "<xmlpatherr>/idcdata/xmltodb/viperr</xmlpatherr>"\
        "<timetvl>5</timetvl>"\
        "<timeout>50</timeout>"\
        "<pname>xmltodb_vip</pname>\"\n"
    );
    printf("本程序是共享平台的公共功能模块, 用于把 xml 文件入库到 Oracle 的表中;\n");
    printf("logfilename: 本程序运行的日志文件;\n");
    printf("xmlbuffer: 本程序运行的参数, 用 xml 表示, 具体如下: \n\n");
    printf("connstr: 数据库的连接参数, 格式: username/passwd@tnsname;\n");
    printf("charset: 数据库的字符集, 这个参数要与数据源数据库保持一致, 否则会出现中文乱码的情况;\n");
    printf("inifilename: 数据入库的参数配置文件;\n");
    printf("xmlpath: 待入库 xml 文件存放的目录;\n");
    printf("xmlpathbak: xml 文件入库后的备份目录;\n");
    printf("xmlpatherr: 入库失败的 xml 文件存放的目录;\n");
    printf("timetvl: 扫描 xmlpath 目录的时间间隔(执行入库任务的时间间隔), 单位: 秒, 视业务需求而定, 2-30之间;\n");
    printf("timeout: 本程序的超时时间, 单位: 秒, 视xml文件大小而定, 建议设置 30 以上;\n");
    printf("pname: 进程名, 尽可能采用易懂的、与其它进程不同的名称, 方便故障排查。\n");
}

// 把 xml 解析到参数 starg 结构体中
bool parseXMLToArg(const char* strxmlbuffer) {
    memset(&starg, 0, sizeof(StArg));

    getxmlbuffer(strxmlbuffer, "connstr", starg.connstr, 100);
    if (strlen(starg.connstr) == 0) {
        logfile.write("connstr is null.\n");
        return false;
    }

    getxmlbuffer(strxmlbuffer, "charset", starg.charset, 50);
    if (strlen(starg.charset) == 0) {
        logfile.write("charset is null.\n");
        return false;
    }

    getxmlbuffer(strxmlbuffer, "inifilename", starg.inifilename, 300);
    if (strlen(starg.inifilename) == 0) {
        logfile.write("inifilename is null.\n");
        return false;
    }

    getxmlbuffer(strxmlbuffer, "xmlpath", starg.xmlpath, 300);
    if (strlen(starg.xmlpath) == 0) {
        logfile.write("xmlpath is null.\n");
        return false;
    }

    getxmlbuffer(strxmlbuffer, "xmlpathbak", starg.xmlpathbak, 300);
    if (strlen(starg.xmlpathbak) == 0) {
        logfile.write("xmlpathbak is null.\n");
        return false;
    }

    getxmlbuffer(strxmlbuffer, "xmlpatherr", starg.xmlpatherr, 300);
    if (strlen(starg.xmlpatherr) == 0) { logfile.write("xmlpatherr is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer, "timetvl", starg.timetvl);
    if (starg.timetvl < 2) {
        starg.timetvl = 2;
    }
    if (starg.timetvl > 30) {
        starg.timetvl = 30;
    }

    getxmlbuffer(strxmlbuffer, "timeout", starg.timeout);
    if (starg.timeout == 0) {
        logfile.write("timeout is null.\n");
        return false;
    }

    getxmlbuffer(strxmlbuffer, "pname", starg.pname, 50);
    if (strlen(starg.pname) == 0) {
        logfile.write("pname is null.\n");
        return false;
    }

    return true;
}

// 业务处理主函数
bool xmlToDatabaseMain() {
    cdir dir;       // 获取目录及其子目录文件列表对象

    int count = 30;     // 循环计数器，控制程序重新加载入库参数配置文件的时间 count * timetvl

    // 每循环一次，执行一次入库任务
    while (true) {

        // 把数据入库的参数配置文件 starg.inifilename 加载到 v_xmltotable 容器中
        if (count == 30) {
            if (loadXMLToVector() == false) {
                logfile.write("loadXMLToVector() failed.\n");
                return false;
            }
            count = 0;
        }
        else {
            ++count;
        }

        // 打开 starg.xmlpath 目录，为了保证先生成的数据入库，打开目录的时候，应该按文件名排序
        if (dir.opendir(starg.xmlpath, "*.XML", 10000, false, true) == false) {
            logfile.write("dir.opendir(%s) failed.\n", starg.xmlpath);
            return false;
        }

        // xml 文件入库前，执行数据库连接
        if (conn.isopen() == false) {
            if (conn.connecttodb(starg.connstr, starg.charset) != 0) {
                logfile.write("connect database(%s) failed.\n%s\n", starg.connstr, conn.message());
                return false;
            }
            logfile.write("connect database(%s) ok.\n", starg.connstr);
        }

        // dir.size() !=0，说明有文件需要入库，此时程序不应该休眠
        // 反之，没有文件需要入库，程序休眠，防止一直占用 cpu 资源
        if (dir.size() == 0) {
            sleep(starg.timetvl);
        }

        while (true) {
            // 读取目录中的每一个 xml 文件
            if (dir.readdir() == false) {
                break;      // 读取完，跳出循环
            }
            logfile.write("process file(%s) ", dir.m_ffilename.c_str());

            // 处理 xml 文件的子函数
            int ret = xmlToDatabase(dir.m_ffilename, dir.m_filename);

            // 每处理完一个 xml 文件，更新进程心跳信息
            pactive.updateAtime();

            if (ret == 0) {
                // xml 文件入库成功，将其移动到 starg.xmlpathbak 备份目录中
                logfile << "ok(records totalcount = " << totalcount << ", insertcount = " << insertcount
                    << ", updatecount = " << updatecount << ", spendtime = " << timer.elapsed() << ").\n";

                if (xmlToBakErr(dir.m_ffilename, starg.xmlpath, starg.xmlpathbak) == false) {
                    logfile.write("xmltoBakErr(%s) failed.\n", dir.m_ffilename.c_str());
                    return false;
                }
            }
            if (ret == 1 || ret == 3 || ret == 4) {
                /*
                    这三种错误把 xml 文件移动到错误目录中
                    - 1：入库参数不正确
                    - 3：待入库的表不存在
                    - 4：执行入库前的 sql 语句失败
                */
                if (ret == 1) {
                    logfile << "failed, the parameter of operating database is error.\n";
                }
                if (ret == 3) {
                    logfile << "failed, the table(" << stxmltotable.tname << ") is not exist.\n";
                }
                if (ret == 4) {
                    logfile << "failed, execute <execsql>" << stxmltotable.execsql << "</execsql> error.\n";
                }

                // 把 xml 文件移动到 starg.xmlpatherr 参数指定的错误目录中
                if (xmlToBakErr(dir.m_ffilename, starg.xmlpath, starg.xmlpatherr) == false) {
                    logfile.write("xmltoBakErr(%s) failed.\n", dir.m_ffilename.c_str());
                    return false;
                }
            }
            if (ret == 2) {
                // 2: 数据库错误，函数返回，程序将退出
                logfile << "failed, oracle database error.\n";
                return false;
            }
            if (ret == 5) {
                // 5：打开 xml 文件失败，函数返回，程序将退出
                logfile << "failed, open xml file(" << dir.m_ffilename << ") error.\n";
                return false;
            }
        }
        // 处理完目录中所有的 xml 文件之后，更新进程心跳
        pactive.updateAtime();
    }

    return true;
}

// 把数据入库参数的配置文件 starg.inifilename 加载到 v_xmltotable 容器中
bool loadXMLToVector() {
    v_xmltotable.clear();    // 清空容器中的内容

    cifile ifile;   // 读取文件内容对象
    if (ifile.open(starg.inifilename) == false) {
        logfile.write("ifile.open(%s) failed.\n", starg.inifilename);
        return false;
    }
    string strbuffer;
    while (true) {
        if (ifile.readline(strbuffer, "<endl/>") == false) {
            break;
        }

        memset(&stxmltotable, 0, sizeof(StXMLToTable));

        // xml 文件的匹配规则，用逗号分隔
        getxmlbuffer(strbuffer, "filename", stxmltotable.filename, 100);
        // 待入库的表名
        getxmlbuffer(strbuffer, "tname", stxmltotable.tname, 30);
        // 更新标志，1-更新，2-不更新
        getxmlbuffer(strbuffer, "updateflag", stxmltotable.updateflag);
        // 处理 xml 文件之前，执行的 sql 语句
        getxmlbuffer(strbuffer, "execsql", stxmltotable.execsql, 300);

        v_xmltotable.push_back(stxmltotable);
    }

    logfile.write("loadXMLToVector(%s) ok.\n", starg.inifilename);

    // 测试代码
    // for (auto& xmltotable : v_xmltotable) {
    //     printf("%s, %s, %d, %s\n",
    //         xmltotable.filename, xmltotable.tname, xmltotable.updateflag, xmltotable.execsql);
    // }
    return true;
}

// 根据文件名，从 v_xmltotable 容器中查找入库参数，存放在 StXMLToTable 结构体中
bool findOpeDBParameterFromVector(const string& xmlfilename) {
    for (auto& xmltotable : v_xmltotable) {
        if (matchstr(xmlfilename, xmltotable.filename) == true) {
            stxmltotable = xmltotable;
            return true;
        }
    }
    return false;
}

/*
    处理 xml 文件的子函数
    返回值：
    - 0：成功
    - 1：入库参数配置不正确（无法找到 xml 文件需要入库的数据库表）
    - 2：数据库系统有问题，或网络断开，或连接超时（无法得到 xml 文件对应数据库表的字段信息和主键）
    - 3：xml 入库对应的数据库表不存在（即程序中，存储数据库表全部字段的容器大小为 0，也有可能入库参数配置错误）
    - 4：执行删除数据库内容的 sql 语句失败，即入库参数配置文件的 <execsql></execsql> 项内容，如果有这项，会在 xml 入库前执行该 sql 语句
    - 5：打开 xml 文件失败，无法执行入库操作
*/
int xmlToDatabase(const string& fullfilename, const string& filename) {

    timer.start();      // 处理 xml 文件开始计时
    totalcount = 0;
    insertcount = 0;
    updatecount = 0;

    // 1. 根据待入库的文件名，查找入库参数，得到对应的表名
    if (findOpeDBParameterFromVector(filename) == false) {
        return 1;
    }

    // 2. 根据表名，读取数据字典，得到表的字段名和主键
    if (tcols.allCols(conn, stxmltotable.tname) == false) {
        return 2;
    }
    if (tcols.pkCols(conn, stxmltotable.tname) == false) {
        return 2;
    }

    // 3. 根据表的字段名和主键，拼接插入和更新的 SQL 语句，并且绑定输入变量
    if (tcols.v_all_cols.size() == 0) {
        // tcols.v_all_cols.size()==0 说明表根本不存在（配错了参数或忘了建表）
        return 3;
    }

    // 3.1 拼接插入和更新的 sql 语句
    jointSQL();

    // 3.2 绑定输入变量，准备待执行的 sql 语句
    prepareSQL();

    // 4. 打开 xml 文件

    // 4.1 在打开 xml 文件之前，如果 stxmltotable.execsql 不为空，就执行它
    if (execsql() == false) {
        return 4;
    }

    // 4.2 打开 xml 文件
    cifile ifile;
    if (ifile.open(fullfilename) == false) {
        // 如果文件打开失败，回滚事务，防止 4.1 执行的 stxmltotable.execsql 造成的数据库信息丢失
        conn.rollback();
        return 5;
    }

    string strbuffer;   // 存放从 xml 文件中读取的一行

    while (true) {
        // 5. 从 xml 文件中读取一行数据
        if (ifile.readline(strbuffer, "<endl/>") == false) {
            break;
        }

        ++totalcount;    // 记录 xml 文件的记录数

        // 6. 根据表的字段名，从 xml 文件中解析出每个字段数据
        splitXMLBuffer(strbuffer);

        // 7. 执行插入或者更新的 sql 语句
        if (stmt_insert.execute() != 0) {
            if (stmt_insert.rc() == 1) {
                // 违反唯一性约束，表示记录已存在
                if (stxmltotable.updateflag == 1) {
                    // 判断入库参数的更新标志，只有 stxmltotable.updateflag==1 才执行 update 语句
                    if (stmt_update.execute() != 0) {
                        // update 失败了，记录出错的行和错误原因

                        // 如果失败原因是数据本身的问题，例如时间格式不正确，数值不合法，数值太大，函数不用返回错误值，不处理这一行数据
                        logfile.write("%s\n", strbuffer.c_str());
                        logfile.write("stmt_update.execute() failed.\n%s\n%s\n", stmt_update.sql(), stmt_update.message());

                        // 如果是数据库系统问题，则函数返回错误代码
                        // ORA-03113：通信通道的文件结尾
                        // ORA-03114：未连接到 oracle 数据库
                        // ORA-03135：oracle 数据库连接失去联系
                        // ORA-16014：归档失败
                        if ((stmt_update.rc() == 3113) || (stmt_update.rc() == 3114) ||
                            (stmt_update.rc() == 3135) || (stmt_update.rc() == 16014)) {
                            return 2;
                        }
                    }
                    else {
                        ++updatecount;      // 更新的记录数加1
                    }
                }
            }
            else {
                // 除了违反唯一性约束的其它错误，insert 失败了，记录出错的行和错误原因

                // 如果失败原因是数据本身的问题，例如时间格式不正确，数值不合法，数值太大，函数不用返回错误值，不处理这一行数据
                logfile.write("\n%s\n", strbuffer.c_str());
                logfile.write("stmt_insert.execute() failed.\n%s\n%s\n", stmt_insert.sql(), stmt_insert.message());

                // 如果是数据库系统问题，则函数返回错误代码
                // ORA-03113：通信通道的文件结尾
                // ORA-03114：未连接到 oracle 数据库
                // ORA-03135：oracle 数据库连接失去联系
                // ORA-16014：归档失败，比如 oracle 数据库对应的磁盘空间不足等
                if ((stmt_insert.rc() == 3113) || (stmt_insert.rc() == 3114) ||
                    (stmt_insert.rc() == 3135) || (stmt_insert.rc() == 16014)) {
                    return 2;
                }
            }
        }
        else {
            ++insertcount;      // 插入的记录数加1
        }
    }

    // 8. 提交事务
    conn.commit();
    return 0;
}

// 拼接 sql 语句
void jointSQL() {

    string str_insertp1;    // insert 语句的字段列表
    string str_insertp2;    // insert 语句 values 后的内容
    int colseq = 1;   // values 部分字段的序号，实现动态 sql

    // 遍历存储表全部字段的容器
    for (auto& cur_col : tcols.v_all_cols) {
        // uptime 字段的缺省值是 sysdate，不需要处理
        if (strcmp(cur_col.colname, "uptime") == 0) {
            continue;
        }

        // 拼接 insert 语句字段列表
        str_insertp1 = str_insertp1 + cur_col.colname + ",";

        // 拼接 values 部分字段，需要区分 keyid, date 和 非 date字段
        if (strcmp(cur_col.colname, "keyid") == 0) {
            // keyid 字段需要特殊处理，stxmltotable.tname+2 是因为表名前两个字符是 'T_' 
            str_insertp2 = str_insertp2 + sformat("SEQ_%s.nextval", stxmltotable.tname + 2) + ",";
        }
        else {
            if (strcmp(cur_col.datatype, "date") == 0) {
                str_insertp2 = str_insertp2 + sformat("to_date(:%d,'yyyymmddhh24miss')", colseq) + ",";
            }
            else {
                str_insertp2 = str_insertp2 + sformat(":%d", colseq) + ",";
            }
            // keyid 使用的是 oracle 序列生成器的值，不需要绑定，所以 colseq 不需要加 1，其它数据 colseq 需要加 1
            ++colseq;
        }
    }
    // 删除最后一个多余的','
    deleterchr(str_insertp1, ',');
    deleterchr(str_insertp2, ',');

    // 拼接出完整的插入 sql 语句
    sformat(str_insertsql, "insert into %s(%s) values(%s)",
        stxmltotable.tname, str_insertp1.c_str(), str_insertp2.c_str());

    //logfile << "str_insertsql = " << str_insertsql << "\n";

    // 如果入库参数中指定了表数据不需要更新，就不拼接 update 语句了，函数直接返回
    if (stxmltotable.updateflag != 1) {
        return;
    }



    // 1. 拼接 update 语句开始的部分
    str_updatesql = sformat("update %s set ", stxmltotable.tname);

    // 2. 拼接 update 语句 set 后面的部分
    colseq = 1;
    // 遍历存储表全部字段的容器
    for (auto& cur_col : tcols.v_all_cols) {
        // 主键字段不需要更新，所以不拼接在 set 后面
        if (cur_col.pkseq != 0) {
            continue;
        }

        // 如果字段名是 keyid，其中的值是 oracle 序列生成器得出的，不需要 update
        if (strcmp(cur_col.colname, "keyid") == 0) {
            continue;
        }

        // 如果字段名是 uptime ，字段直接赋值 sysdate
        if (strcmp(cur_col.colname, "uptime") == 0) {
            str_updatesql = str_updatesql + "uptime=sysdate,";
            continue;
        }

        // 其它字段拼接在 set 后面，需要区分 date 字段和非 date 字段
        if (strcmp(cur_col.datatype, "date") == 0) {
            // date 字段
            str_updatesql = str_updatesql + sformat("%s=to_date(:%d,'yyyymmddhh24miss'),", cur_col.colname, colseq);
        }
        else {
            // 非 date 字段
            str_updatesql = str_updatesql + sformat("%s=:%d,", cur_col.colname, colseq);
        }

        ++colseq;   // 绑定遍历的序号加 1

    }

    deleterchr(str_updatesql, ',');     // 删除最后一个多余的逗号

    // 3. 拼接 update 语句 where 后面的部分
    str_updatesql = str_updatesql + " where 1=1";
    for (auto& cur_col : tcols.v_all_cols) {
        if (cur_col.pkseq == 0) {
            continue;   // 非主键字段，不作为更新的条件
        }
        if (strcmp(cur_col.datatype, "date") == 0) {
            // 主键是 date 字段
            str_updatesql = str_updatesql + sformat(" and %s=to_date(:%d,'yyyymmddhh24miss')", cur_col.colname, colseq);
        }
        else {
            // 非 date 字段
            str_updatesql = str_updatesql + sformat(" and %s=:%d", cur_col.colname, colseq);
        }
        colseq++;
    }

    //logfile.write("str_updatesql = %s\n", str_updatesql.c_str());

    return;
}

// 动态 sql 绑定变量
void prepareSQL() {
    // 为输入变量的数组 v_colvalue 分配内存
    v_colvalue.resize(tcols.v_all_cols.size());

    // 准备插入表的 sql 语句、绑定输入变量
    stmt_insert.connect(&conn);
    stmt_insert.prepare(str_insertsql);

    int colseq = 1;   // 输入变量（参数），values 部分字段的序号

    // 遍历全部字段的容器
    for (int i = 0;i < tcols.v_all_cols.size();++i) {
        // uptime 和 keyid 这两个字段不需要绑定参数
        if ((strcmp(tcols.v_all_cols[i].colname, "uptime") == 0) ||
            (strcmp(tcols.v_all_cols[i].colname, "keyid") == 0)) {
            continue;
        }

        // 绑定输入参数
        stmt_insert.bindin(colseq, v_colvalue[i], tcols.v_all_cols[i].collen);
        //logfile.write("stmt_insert.bindin(%d, v_colvalue[%d],%d);\n", colseq, i, tcols.v_all_cols[i].collen);
        ++colseq;
    }

    // 准备更新表的 sql 语句、绑定输入变量
    // 如果入库参数中指定了表的数据不需要更新，就不处理 update 语句了
    if (stxmltotable.updateflag != 1) {
        return;
    }
    stmt_update.connect(&conn);
    stmt_update.prepare(str_updatesql);

    colseq = 1;     // 输入变量（参数），set 和 where 部分字段的序号

    // 绑定 set 部分的输入参数
    for (int i = 0;i < tcols.v_all_cols.size();++i) {
        // 主键字段，不在更新语句的 set 后面
        if (tcols.v_all_cols[i].pkseq != 0) {
            continue;
        }
        // uptime 和 keyid 这两个字段不需要处理
        if ((strcmp(tcols.v_all_cols[i].colname, "uptime") == 0) ||
            (strcmp(tcols.v_all_cols[i].colname, "keyid") == 0)) {
            continue;
        }

        // 绑定输入参数
        stmt_update.bindin(colseq, v_colvalue[i], tcols.v_all_cols[i].collen);
        //logfile.write("stmt_update.bindin(%d, v_colvalue[%d],%d);\n", colseq, i, tcols.v_all_cols[i].collen);
        ++colseq;
    }

    // 绑定 where 部分的输入参数
    for (int i = 0;i < tcols.v_all_cols.size();++i) {
        // 非主键字段，不在更新语句的 where 后面
        if (tcols.v_all_cols[i].pkseq == 0) {
            continue;
        }

        // 绑定输入参数
        stmt_update.bindin(colseq, v_colvalue[i], tcols.v_all_cols[i].collen);
        //logfile.write("stmt_update.bindin(%d, v_colvalue[%d],%d);\n", colseq, i, tcols.v_all_cols[i].collen);
        ++colseq;
    }

    return;
}

// 在打开 xml 文件之前，如果 stxmltotable.execsql 不为空，就执行它
bool execsql() {
    if (strlen(stxmltotable.execsql) == 0) {
        return true;
    }
    sqlstatement stmt(&conn);
    stmt.prepare(stxmltotable.execsql);
    if (stmt.execute() != 0) {
        logfile.write("\nstmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return false;
    }

    return true;
}

// 解析 xml 文件中读取的每一行数据，存放在已绑定的输入变量 v_colvalue 数组中
void splitXMLBuffer(const string& strbuffer) {
    string strtmp;  // 临时变量，存放从 xml 中解析出来的字段值

    for (int i = 0;i < tcols.v_all_cols.size();++i) {
        // 根据字段名，从 xml 中把数据项的值解析出来，存放在临时变量 strtmp 中
        // 用临时变量是为了防止调用移动构造函数和移动赋值函数改变 v_colvalue 数组中 string 的内部地址
        getxmlbuffer(strbuffer, tcols.v_all_cols[i].colname, strtmp, tcols.v_all_cols[i].collen);

        // 如果是日期时间字段 date，提取数字就行了
        // 也就是说， xml 文件中的日期时间只要包含了 yyyymmddhh24miss 就行，可以是任意分隔符
        if (strcmp(tcols.v_all_cols[i].datatype, "date") == 0) {
            // 提取字符串中的数字、符号和小数点，后面两个 false 分别表示不提取符号和小数点
            picknumber(strtmp, strtmp, false, false);
        }
        else if (strcmp(tcols.v_all_cols[i].datatype, "number") == 0) {
            // 如果是数值字段 number，只提取数字、符号和小数点
            picknumber(strtmp, strtmp, true, true);
        }

        // 如果是字符字段，不需要做任何处理
        v_colvalue[i] = strtmp;       // 不能采用这行代码，会调用移动赋值函数，但是 g++ 11.3 不会出现问题
        //v_colvalue[i] = strtmp.c_str();
    }

    return;
}

// 把 xml 文件移动到备份目录或错误目录
bool xmlToBakErr(const string& fullfilename, const string& srcpath, const string& dstpath) {
    string dstfilename = fullfilename;
    // 字符串替换函数，第四个参数 false 表示不执行循环替换，因为 dstfilename 中不存在多段 srcpath
    replacestr(dstfilename, srcpath, dstpath, false);

    if (renamefile(fullfilename, dstfilename.c_str()) == false) {
        logfile.write("renamefile(%s,%s) failed.\n", fullfilename, dstfilename.c_str());
        return false;
    }

    return true;
}

// 程序退出的信号处理函数
void EXIT(int sig) {
    logfile.write("program exit, sig = %d.\n", sig);
    exit(0);
}