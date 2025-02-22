/*
    程序功能：数据库中表记录的迁移
*/
#include"_tools.h"
using namespace idc;

// 程序运行参数的结构体
typedef struct StArg {
    char connstr[101];      // 数据库的连接参数
    char tname[31];         // 待迁移的表名
    char totname[31];       // 目的表名
    char keycol[31];        // 待迁移表的唯一键字段名
    char where[1001];       // 待迁移数据需要满足的条件
    int maxcount;           // 执行一次 SQL 删除的记录数
    char starttime[31];     // 程序运行的时间区间
    int timeout;            // 程序运行的超时时间
    char pname[51];         // 程序名
}StArg;

clogfile logfile;
StArg starg;
connection conn;
Cpactive pactive;

// 程序帮助文档
void Help();

// 判断当前程序是否在程序运行的时间区间内
bool instarttime();

// 把 xml 解析到参数 starg 结构体变量中
bool xmlToArg(const char* strxmlbuffer);

// 业务处理主函数
bool migrateTable();

// 程序退出和信号 2, 15 的处理函数
void EXIT(int sig);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        Help();
        return -1;
    }

    // 关闭全部的信号和输入输出
    //closeioandsignal();
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    if (logfile.open(argv[1]) == false) {
        printf("open logfile(%s) failed.\n", argv[1]);
        return -1;
    }

    // 把 xml 文件解析到参数 starg 结构体变量中
    if (xmlToArg(argv[2]) == false) {
        logfile.write("parse xml to starg failed.\n");
        return -1;
    }

    // 判断当前时间是否在程序运行的时间区间内
    if (instarttime() == false) {
        return 0;
    }

    // 注意，在程序调试时，可以启用类似以下的代码，防止超时
    pactive.addProcInfo(starg.timeout * 10000, starg.pname);

    // 数据库连接
    if (conn.connecttodb(starg.connstr, "Simplified Chinese_China.AL32UTF8") != 0) {
        logfile.write("connect database(%s) failed.\n%s\n", starg.connstr, conn.message());
        return -1;
    }
    logfile.write("connect database(%s) ok.\n", starg.connstr);

    migrateTable();

    return 0;
}

// 程序帮助文档
void Help() {
    printf("Using: /CppIndustrialMeteorologyProject/tools/bin/migratetable logfilename xmlbuffer\n");
    printf("Sample:\n"\
        "/CppIndustrialMeteorologyProject/tools/bin/procctl 3600 "\
        "/CppIndustrialMeteorologyProject/tools/bin/migratetable "\
        "/log/idc/migratetable_ZHOBTMIND1.log "\
        "\"<connstr>idc/idcpwd</connstr>"\
        "<tname>T_ZHOBTMIND1</tname>"\
        "<totname>T_ZHOBTMIND1_HIS</totname>"\
        "<keycol>rowid</keycol>"\
        "<where>where ddatetime<sysdate-2/24</where>"\
        "<maxcount>10</maxcount>"\
        "<starttime></starttime>"\
        "<timeout>120</timeout>"\
        "<pname>migratetable_ZHOBTMIND1</pname>\"\n");

    printf("本程序是共享平台的公共功能模块, 用于迁移表中的数据;\n");
    printf("logfilename: 本程序运行的日志文件;\n");
    printf("xmlbuffer: 本程序运行的参数, 用xml表示, 具体如下: \n\n");
    printf("connstr: 数据库的连接参数, 格式: username/passwd@tnsname;\n");
    printf("tname: 待迁移数据表的表名;\n");
    printf("totname: 目的表名，例如 T_ZHOBTMIND1_HIS;\n");
    printf("keycol: 待迁移数据表的唯一键字段名, 可以用记录编号, 如keyid, 建议用rowid, 效率最高;\n");
    printf("where: 待迁移的数据需要满足的条件, 即SQL语句中的where部分;\n");
    printf("maxcount: 执行一次SQL语句删除的记录数, 建议在 100-500 之间;\n");
    printf("starttime: 程序运行的时间区间, 例如 [02, 13] 表示: 如果程序运行时, 在 02 时到 13 时之间则运行, 其它时间不运行,"\
        "如果 starttime 为空, 本参数将失效, 只要本程序启动就会执行数据迁移, "\
        "为了减少对数据库的压力, 数据迁移一般在业务最闲的时候时进行;\n");
    printf("timeout: 本程序的超时时间, 单位: 秒, 建议设置 120 以上;\n");
    printf("pname: 进程名, 尽可能采用易懂的、与其它进程不同的名称, 方便故障排查。\n");
}

// 把 xml 解析到参数 starg 结构体变量中
bool xmlToArg(const char* strxmlbuffer) {
    memset(&starg, 0, sizeof(StArg));

    getxmlbuffer(strxmlbuffer, "connstr", starg.connstr, 100);
    if (strlen(starg.connstr) == 0) {
        logfile.write("connstr is null.\n");
        return false;
    }

    getxmlbuffer(strxmlbuffer, "tname", starg.tname, 30);
    if (strlen(starg.tname) == 0) {
        logfile.write("tname is null.\n");
        return false;
    }

    getxmlbuffer(strxmlbuffer, "totname", starg.totname, 30);
    if (strlen(starg.totname) == 0) {
        logfile.write("totname is null.\n");
        return false;
    }

    getxmlbuffer(strxmlbuffer, "keycol", starg.keycol, 30);
    if (strlen(starg.keycol) == 0) {
        logfile.write("keycol is null.\n");
        return false;
    }

    getxmlbuffer(strxmlbuffer, "where", starg.where, 1000);
    if (strlen(starg.where) == 0) {
        logfile.write("where is null.\n");
        return false;
    }

    getxmlbuffer(strxmlbuffer, "starttime", starg.starttime, 30);

    getxmlbuffer(strxmlbuffer, "maxcount", starg.maxcount);
    if (starg.maxcount == 0) {
        logfile.write("maxcount is null.\n");
        return false;
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

// 判断当前程序是否在程序运行的时间区间内
bool instarttime() {
    // starttime 为空，默认不启用时间区间判断
    if (strlen(starg.starttime) != 0) {
        // 获取操作系统时间的小时部分，判断是否在区间 [02,13] 内，一般的数据库，闲时时间为 12 - 14 和 00 - 06 时
        string strhh24 = ltime1("hh24");
        if (strstr(starg.starttime, strhh24.c_str()) == 0) {
            logfile.write("programming isn't in running time.\n");
            return false;
        }
    }
    return true;
}


// 业务处理主函数
bool migrateTable() {
    ctimer timer;
    char tmpvalue[21];      // 存放待删除记录的唯一键值，绑定 select 语句的输出

    // 1. 准备从表中提取数据的 SQL 语句
    // select rowid from T_ZHOBTMIND1 where ddatetime < sysdate - 1
    sqlstatement stmt_select(&conn);
    stmt_select.prepare("select %s from %s %s", starg.keycol, starg.tname, starg.where);
    stmt_select.bindout(1, tmpvalue, 20);

    // 2. 准备从表中删除数据的 SQL 语句
    // delete from T_ZHOBTMIND1 where rowid in (:1,:2,:3,:4,:5,:6,:7,:8,:9,:10);
    string deletesql = sformat("delete from %s where %s in (", starg.tname, starg.keycol);
    for (int i = 0;i < starg.maxcount;++i) {
        deletesql = deletesql + sformat(":%lu,", i + 1);    // %lu: unsigned long
    }

    deleterchr(deletesql, ',');
    deletesql = deletesql + ")";

    char keyvalues[starg.maxcount][21];     // 存放唯一键字段值的数组，绑定 delete 和 insert 语句的输入

    sqlstatement stmt_delete(&conn);
    stmt_delete.prepare(deletesql);         // 准备删除数据的 sql 语句

    for (int i = 0;i < starg.maxcount;++i) {
        stmt_delete.bindin(i + 1, keyvalues[i], 20);
    }

    // 3. 准备插入目的表的 sql 语句，绑定输入参数
    ctcols tcols;
    tcols.allCols(conn, starg.tname);

    // 注意：select 后面最好不要用 *，防止插入表和待插入表的字段顺序不一致
    string insertsql = sformat("insert into %s select %s from %s where %s in (", starg.totname, tcols.str_all_cols.c_str(),
        starg.tname, starg.keycol);
    // 拼接插入目的表的 sql 语句
    for (int i = 0;i < starg.maxcount;++i) {
        insertsql = insertsql + sformat(":%lu,", i + 1);
    }
    deleterchr(insertsql, ',');
    insertsql = insertsql + ")";

    sqlstatement stmt_insert(&conn);
    stmt_insert.prepare(insertsql);

    for (int i = 0;i < starg.maxcount;++i) {
        stmt_insert.bindin(i + 1, keyvalues[i], 20);
    }

    if (stmt_select.execute() != 0) {
        logfile.write("stmt_select.execute() failed.\n%s\n%s\n", stmt_select.sql(), stmt_select.message());
        return false;
    }

    int ccount = 0;         // keyvalues 数组中有效元素的个数
    memset(keyvalues, 0, sizeof(keyvalues));

    // 4. 处理结果集
    while (true) {
        // 3.1 从结果集中获取一行记录，放在临时数组中
        memset(tmpvalue, 0, sizeof(tmpvalue));
        if (stmt_select.next() != 0) {
            break;
        }

        strcpy(keyvalues[ccount], tmpvalue);
        ++ccount;

        // 3.2 如果数组中记录数达到了 starg.maxcount，执行一次删除数据的 SQL 语句
        if (ccount == starg.maxcount) {
            // 先执行迁移数据的 sql 语句，然后再执行删除数据的 sql 语句
            if (stmt_insert.execute() != 0) {
                logfile.write("stmt_insert.execute() failed.\n%s\n%s\n", stmt_insert.sql(), stmt_insert.message());
                return false;
            }
            if (stmt_delete.execute() != 0) {
                logfile.write("stmt_delete.execute() failed.\n%s\n%s\n", stmt_delete.sql(), stmt_delete.message());
                return false;
            }

            // 手动提交事务
            conn.commit();

            ccount = 0;
            memset(keyvalues, 0, sizeof(keyvalues));

            // 执行一次删除记录，更新进程心跳
            pactive.updateAtime();
        }
    }

    // 4. 如果临时数组中有记录，再执行一次删除数据的 SQL 语句
    if (ccount > 0) {
        if (stmt_insert.execute() != 0) {
            logfile.write("stmt_insert.execute() failed.\n%s\n%s\n", stmt_insert.sql(), stmt_insert.message());
            return false;
        }
        if (stmt_delete.execute() != 0) {
            logfile.write("stmt_delete.execute() failed.\n%s\n%s\n", stmt_delete.sql(), stmt_delete.message());
            return false;
        }

        // 手动提交事务
        conn.commit();
    }

    // 把删除的记录数写日志
    if (stmt_select.rpc() > 0) {
        logfile.write("migrate from %s to %s %d rows in %.02f sec.\n", starg.tname, starg.totname,
            stmt_select.rpc(), timer.elapsed());
    }

    return true;
}

// 程序退出和信号 2, 15 的处理函数
void EXIT(int sig) {
    logfile.write("program exit, sig = %d.\n", sig);
    conn.disconnect();
    exit(0);
}