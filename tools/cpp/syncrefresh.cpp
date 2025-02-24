/*
    程序功能，采用刷新同步的方法同步 Oracle 数据库的远程表
*/
#include "_tools.h"

// 程序运行参数的结构体
typedef struct StArg {
    char localconnstr[101];     // 本地数据库的连接参数
    char charset[51];           // 数据库的字符集
    char linktname[31];         // dblink指向的远程表名，如T_ZHOBTCODE1@local
    char localtname[31];        // 本地表名
    char remotecols[1001];      // 远程表的字段列表
    char localcols[1001];       // 本地表的字段列表
    char rwhere[1001];          // 同步数据的条件
    char lwhere[1001];          // 同步数据的条件
    int synctype;               // 同步方式：1-不分批刷新；2-分批刷新
    char remoteconnstr[101];    // 远程数据库的连接参数
    char remotetname[31];       // 远程表名
    char remotekeycol[31];      // 远程表的键值字段名
    char localkeycol[31];       // 本地表的键值字段名
    int keylen;                 // 键值字段的长度
    int maxcount;               // 执行一次同步操作的记录数
    int timeout;                // 本程序运行时的超时时间
    char pname[51];             // 本程序运行时的程序名
} StArg;

// 程序运行参数结构体对象
StArg starg;
// 进程心跳
Cpactive pactive;
// 日志文件对象
clogfile logfile;

// 本地数据库连接
connection connlocal;
// 分批刷新（同步），需要远程数据库连接，这样可以不使用 dblink，提高了查询效率
connection connremote;

// 显示程序的帮助
void Help();

// 把xml解析到参数starg结构中
bool xmlToArg(const char* strxmlbuffer);

// 业务处理主函数。
bool syncRefresh();

// 程序退出
void EXIT(int sig);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        Help();
        return -1;
    }

    // 关闭全部的信号和输入输出，处理程序退出的信号
    //closeioandsignal(true); 
    signal(SIGINT, EXIT); signal(SIGTERM, EXIT);

    if (logfile.open(argv[1]) == false) {
        printf("open logfile(%s) failed.\n", argv[1]);
        return -1;
    }

    // 把xml解析到参数starg结构中
    if (xmlToArg(argv[2]) == false) {
        logfile.write("parse xmlfile(%s) failed.\n", argv[2]);
        return -1;
    }

    // pactive.addpinfo(starg.timeout,starg.pname);
    // 在调试程序的时候，可以启用类似以下的代码，防止超时
    pactive.addProcInfo(starg.timeout * 10000, starg.pname);

    if (connlocal.connecttodb(starg.localconnstr, starg.charset) != 0) {
        logfile.write("connect database(%s) failed.\n%s\n", starg.localconnstr, connlocal.message());
        EXIT(-1);
    }
    logfile.write("connect local database(%s) ok.\n", starg.localconnstr);

    // 如果 starg.remotecols 或 starg.localcols 为空，就用 starg.localtname 表的全部字段来填充
    if ((strlen(starg.remotecols) == 0) || (strlen(starg.localcols) == 0)) {
        ctcols tcols;

        // 获取 starg.localtname 表的全部字段（查询数据字典）
        if (tcols.allCols(connlocal, starg.localtname) == false) {
            logfile.write("table(%s) is not exist.\n", starg.localtname);
            EXIT(-1);
        }

        if (strlen(starg.localcols) == 0) {
            strcpy(starg.localcols, tcols.str_all_cols.c_str());
        }

        if (strlen(starg.remotecols) == 0) {
            strcpy(starg.remotecols, tcols.str_all_cols.c_str());
        }
    }

    // 业务处理主函数
    syncRefresh();
}

// 显示程序的帮助
void Help()
{
    printf("Using: /CppIndustrialMeteorologyProject/tools/bin/syncrefresh logfilename xmlbuffer\n");

    printf("不分批同步, 把 T_ZHOBTCODE1@local 同步到 T_ZHOBTCODE2: \n");

    printf("Sample:\n/CppIndustrialMeteorologyProject/tools/bin/procctl 10 "\
        "/CppIndustrialMeteorologyProject/tools/bin/syncrefresh "\
        "/log/idc/syncrefresh_ZHOBTCODE2.log "\
        "\"<localconnstr>idc1/idcpwd1</localconnstr>"\
        "<charset>Simplified Chinese_China.AL32UTF8</charset>"\
        "<linktname>T_ZHOBTCODE1@local</linktname>"\
        "<localtname>T_ZHOBTCODE2</localtname>"\
        "<remotecols>obtid,cityname,provname,latitude,longitude,height,uptime,keyid</remotecols>"\
        "<localcols>stid,cityname,provname,latitude,longitude,height,uptime,recid</localcols>"\
        "<rwhere>where obtid like '57%%%%'</rwhere>"\
        "<lwhere>where stid like '57%%%%'</lwhere>"\
        "<synctype>1</synctype>"\
        "<timeout>50</timeout>"\
        "<pname>syncrefresh_ZHOBTCODE2</pname>\"\n");

    printf("\n分批同步, 把 T_ZHOBTCODE1@local 同步到 T_ZHOBTCODE3: \n");

    printf("因为测试的需要, xmltodb 程序每次会删除 T_ZHOBTCODE1@local 中的数据, 全部的记录重新入库, keyid会变;\n");
    printf("所以, 以下脚本不能用 keyid, 要用 obtid, 用 keyid 会出问题, 可以试试;\n");
    printf("/CppIndustrialMeteorologyProject/tools/bin/procctl 10 "\
        "/CppIndustrialMeteorologyProject/tools/bin/syncrefresh "\
        "/log/idc/syncrefresh_ZHOBTCODE3.log "\
        "\"<localconnstr>idc1/idcpwd1</localconnstr>"\
        "<charset>Simplified Chinese_China.AL32UTF8</charset>"\
        "<linktname>T_ZHOBTCODE1@local</linktname>"\
        "<localtname>T_ZHOBTCODE3</localtname>"\
        "<remotecols>obtid,cityname,provname,latitude,longitude,height,uptime,keyid</remotecols>"\
        "<localcols>stid,cityname,provname,latitude,longitude,height,uptime,recid</localcols>"\
        "<rwhere>where obtid like '5%%%%'</rwhere>"\
        "<synctype>2</synctype>"\
        "<remoteconnstr>idc/idcpwd</remoteconnstr>"\
        "<remotetname>T_ZHOBTCODE1</remotetname>"\
        "<remotekeycol>obtid</remotekeycol>"\
        "<localkeycol>stid</localkeycol>"\
        "<keylen>5</keylen>"\
        "<maxcount>10</maxcount>"\
        "<timeout>50</timeout>"\
        "<pname>syncrefresh_ZHOBTCODE3</pname>\"\n");

    printf("\n分批同步, 把 T_ZHOBTMIND1@local 同步到 T_ZHOBTMIND2;\n");
    printf("/CppIndustrialMeteorologyProject/tools/bin/procctl 10 "\
        "/CppIndustrialMeteorologyProject/tools/bin/syncrefresh "\
        "/log/idc/syncrefresh_ZHOBTMIND2.log "\
        "\"<localconnstr>idc1/idcpwd1</localconnstr>"\
        "<charset>Simplified Chinese_China.AL32UTF8</charset>"\
        "<linktname>T_ZHOBTMIND1@local</linktname>"\
        "<localtname>T_ZHOBTMIND2</localtname>"\
        "<remotecols>obtid,ddatetime,t,p,rh,wd,ws,r,vis,uptime,keyid</remotecols>"\
        "<localcols>stid,ddatetime,t,p,rh,wd,ws,r,vis,uptime,recid</localcols>"\
        "<rwhere></rwhere>"\
        "<synctype>2</synctype>"\
        "<remoteconnstr>idc/idcpwd</remoteconnstr>"\
        "<remotetname>T_ZHOBTMIND1</remotetname>"\
        "<remotekeycol>keyid</remotekeycol>"\
        "<localkeycol>recid</localkeycol>"\
        "<keylen>15</keylen>"\
        "<maxcount>10</maxcount>"\
        "<timeout>50</timeout>"\
        "<pname>syncrefresh_ZHOBTMIND2</pname>\"\n");
    printf("本程序是共享平台的公共功能模块, 采用刷新的方法同步 Oracle 数据库之间的表;\n");
    printf("logfilename: 本程序运行的日志文件;\n");
    printf("xmlbuffer: 本程序运行的参数, 用 xml 表示, 具体如下:\n");
    printf("localconnstr: 本地数据库的连接参数, 格式: username/passwd@tnsname;\n");
    printf("charset: 数据库的字符集, 这个参数要与本地和远程数据库保持一致, 否则会出现中文乱码的情况;\n");
    printf("linktname: dblink 指向的远程表名, 如 T_ZHOBTCODE1@local;\n");
    printf("localtname: 本地表名, 如 T_ZHOBTCODE2;\n");
    printf("remotecols: 远程表的字段列表, 用于填充在 select 和 from 之间, 所以, remotecols 可以是真实的字段, "\
        "也可以是函数的返回值或者运算结果, 如果本参数为空, 就用 localtname 表的字段列表填充;\n");
    printf("localcols: 本地表的字段列表, 与 remotecols 不同, 它必须是真实存在的字段, 如果本参数为空, "\
        "就用 localtname 表的字段列表填充;\n");
    printf("lwhere: 同步数据的条件, 填充在本地表的删除语句之后, 为空则表示同步全部的记录;\n");
    printf("rwhere: 同步数据的条件, 填充在远程表的查询语句之后, 为空则表示同步全部的记录;\n");
    printf("synctype: 同步方式: 1-不分批刷新, 2-分批刷新;\n");
    // 增加远程数据库连接参数，可以不使用 dblink 操作，提高了数据查询的效率
    printf("remoteconnstr: 远程数据库的连接参数, 格式与 localconnstr 相同, 当 synctype==2 时有效;\n");
    printf("remotetname: 没有 dblink 的远程表名, 当 synctype==2 时有效;\n");
    printf("remotekeycol: 远程表的键值字段名, 必须是唯一的, 当 synctype==2 时有效;\n");
    printf("localkeycol: 本地表的键值字段名, 必须是唯一的, 当 synctype==2 时有效;\n");
    printf("keylen: 键值字段的长度, 当 synctype==2 时有效;\n");
    printf("maxcount: 执行一次同步操作的记录数, 当 synctype==2 时有效;\n");
    printf("timeout: 本程序的超时时间, 单位：秒, 视数据量的大小而定, 建议设置 30 以上;\n");
    printf("pname: 本程序运行时的进程名, 尽可能采用易懂的, 与其它进程不同的名称, 方便故障排查;\n");
    printf("注意: \n"\
        "1. remotekeycol 和 localkeycol 字段的选取很重要, 如果是自增字段, 那么在远程表中数据生成后自增字段的值不可改变, 否则同步会失败; \n"\
        "2. 当远程表中存在 delete 操作时, 无法分批刷新, 因为远程表的记录被 delete 后就找不到了, 无法从本地表中执行 delete 操作。\n");
}

// 把xml解析到参数starg结构中
bool xmlToArg(const char* strxmlbuffer)
{
    memset(&starg, 0, sizeof(StArg));

    // 本地数据库的连接参数, 格式：ip, username, password, dbname, port
    getxmlbuffer(strxmlbuffer, "localconnstr", starg.localconnstr, 100);
    if (strlen(starg.localconnstr) == 0) {
        logfile.write("localconnstr is null.\n");
        return false;
    }

    // 数据库的字符集，这个参数要与远程数据库保持一致，否则会出现中文乱码的情况
    getxmlbuffer(strxmlbuffer, "charset", starg.charset, 50);
    if (strlen(starg.charset) == 0) {
        logfile.write("charset is null.\n");
        return false;
    }

    // linktname表名
    getxmlbuffer(strxmlbuffer, "linktname", starg.linktname, 30);
    if (strlen(starg.linktname) == 0) {
        logfile.write("linktname is null.\n");
        return false;
    }

    // 本地表名
    getxmlbuffer(strxmlbuffer, "localtname", starg.localtname, 30);
    if (strlen(starg.localtname) == 0) {
        logfile.write("localtname is null.\n");
        return false;
    }

    /*
        远程表的字段列表，用于填充在 select 和 from 之间，所以，remotecols可以是真实的字段，也可以是函数
        的返回值或者运算结果。如果本参数为空，将用localtname表的字段列表填充。
    */
    getxmlbuffer(strxmlbuffer, "remotecols", starg.remotecols, 1000);

    // 本地表的字段列表，与 remotecols 不同，它必须是真实存在的字段，如果本参数为空，将用localtname表的字段列表填充
    getxmlbuffer(strxmlbuffer, "localcols", starg.localcols, 1000);

    // 同步数据的条件
    getxmlbuffer(strxmlbuffer, "rwhere", starg.rwhere, 1000);
    getxmlbuffer(strxmlbuffer, "lwhere", starg.lwhere, 1000);

    // 同步方式：1-不分批刷新；2-分批刷新
    getxmlbuffer(strxmlbuffer, "synctype", starg.synctype);
    if ((starg.synctype != 1) && (starg.synctype != 2)) {
        logfile.write("synctype is not in (1,2).\n");
        return false;
    }

    if (starg.synctype == 2) {
        // 远程数据库的连接参数，格式与localconnstr相同，当 synctype==2 时有效
        getxmlbuffer(strxmlbuffer, "remoteconnstr", starg.remoteconnstr, 100);
        if (strlen(starg.remoteconnstr) == 0) {
            logfile.write("remoteconnstr is null.\n");
            return false;
        }

        // 远程表名，当 synctype==2 时有效
        getxmlbuffer(strxmlbuffer, "remotetname", starg.remotetname, 30);
        if (strlen(starg.remotetname) == 0) {
            logfile.write("remotetname is null.\n");
            return false;
        }

        // 远程表的键值字段名，必须是唯一的，当 synctype==2 时有效
        getxmlbuffer(strxmlbuffer, "remotekeycol", starg.remotekeycol, 30);
        if (strlen(starg.remotekeycol) == 0) {
            logfile.write("remotekeycol is null.\n");
            return false;
        }

        // 本地表的键值字段名，必须是唯一的，当 synctype==2 时有效
        getxmlbuffer(strxmlbuffer, "localkeycol", starg.localkeycol, 30);
        if (strlen(starg.localkeycol) == 0) {
            logfile.write("localkeycol is null.\n");
            return false;
        }

        // 键值字段的大小
        getxmlbuffer(strxmlbuffer, "keylen", starg.keylen);
        if (starg.keylen == 0) {
            logfile.write("keylen is null.\n");
            return false;
        }

        // 每批执行一次同步操作的记录数，当 synctype==2 时有效
        getxmlbuffer(strxmlbuffer, "maxcount", starg.maxcount);
        if (starg.maxcount == 0) {
            logfile.write("maxcount is null.\n");
            return false;
        }
    }

    // 本程序的超时时间，单位：秒，视数据量的大小而定，建议设置 30 以上
    getxmlbuffer(strxmlbuffer, "timeout", starg.timeout);
    if (starg.timeout == 0) {
        logfile.write("timeout is null.\n");
        return false;
    }

    // 本程序运行时的进程名，尽可能采用易懂的，与其它进程不同的名称，方便故障排查
    getxmlbuffer(strxmlbuffer, "pname", starg.pname, 50);
    if (strlen(starg.pname) == 0) {
        logfile.write("pname is null.\n");
        return false;
    }

    return true;
}

// 业务处理主函数
bool syncRefresh() {
    ctimer timer;
    // 删除本地表中记录的 sql 语句
    sqlstatement stmt_delete(&connlocal);
    // 向本地表中插入数据的 sql 语句
    sqlstatement stmt_insert(&connlocal);

    // 不分批刷新的情况
    if (starg.synctype == 1) {
        logfile.write("sync %s to %s", starg.linktname, starg.localtname);

        // 先删除本地表 starg.localtname 中满足starg.lwhere 条件的记录
        stmt_delete.prepare("delete from %s %s", starg.localtname, starg.lwhere);
        if (stmt_delete.execute() != 0) {
            logfile.write("stmt_delete.execute() failed.\n%s\n%s\n", stmt_delete.sql(), stmt_delete.message());
            return false;
        }

        // 再把远程表 starg.linktname 中满足 starg.rwhere 条件的记录插入到本地表 starg.localtname 中
        stmt_insert.prepare("insert into %s(%s) select %s from %s %s", starg.localtname,
            starg.localcols, starg.remotecols, starg.linktname, starg.rwhere);
        if (stmt_insert.execute() != 0) {
            logfile.write("stmt_insert.execute() failed.\n%s\n%s\n", stmt_insert.sql(), stmt_insert.message());

            // 为了增加程序的可读性，加了回滚事务操作，也可以不用手动回滚事务，connection 类的析构函数会回滚
            connlocal.rollback();
            return false;
        }

        logfile << " " << stmt_insert.rpc() << " rows in " << timer.elapsed() << " sec.\n";

        // 不要忘记手动提交事务
        connlocal.commit();
        return true;
    }
    // 以下是分批刷新的情况
    if (connremote.connecttodb(starg.remoteconnstr, starg.charset) != 0) {
        logfile.write("connect remote database(%s) failed.\n%s\n", starg.remoteconnstr, connremote.message());
        return false;
    }

    logfile.write("connect remote database(%s) ok.\n", starg.remoteconnstr);

    // 从远程表查询需要同步记录的键值
    sqlstatement stmt_select(&connremote);
    stmt_select.prepare("select %s from %s %s", starg.remotekeycol, starg.remotetname, starg.rwhere);
    // 存放远程表查询到需要同步记录的键值
    char remotekeyvalue[starg.keylen + 1];
    stmt_select.bindout(1, remotekeyvalue, 50);

    // 拼接绑定同步 sql 语句参数的字符串(:1,:2,:3,....:starg.maxcount);
    string bindstr;     // 绑定同步 sql 语句参数的字符串
    for (int i = 0;i < starg.maxcount;++i) {
        bindstr = bindstr + sformat(":%lu,", i + 1);
    }
    deleterchr(bindstr, ',');

    char keyvalues[starg.maxcount][starg.keylen + 1];

    // 准备删除本地表数据的 sql 语句，一次删除 starg.maxcount 条记录
    stmt_delete.prepare("delete from %s where %s in (%s)", starg.localtname, starg.localkeycol, bindstr.c_str());
    for (int i = 0;i < starg.maxcount;++i) {
        stmt_delete.bindin(i + 1, keyvalues[i]);
    }

    // 准备插入本地表数据的 sql 语句，一次插入 starg.maxcount 条记录
    stmt_insert.prepare("insert into %s(%s) select %s from %s where %s in (%s)", starg.localtname, starg.localcols,
        starg.remotecols, starg.linktname, starg.remotekeycol, bindstr.c_str());
    for (int i = 0;i < starg.maxcount;++i) {
        stmt_insert.bindin(i + 1, keyvalues[i]);
    }

    // 记录从结果集中已获取记录的计数器
    int ccount = 0;

    memset(keyvalues, 0, sizeof(keyvalues));

    logfile.write("sync %s to %s...", starg.linktname, starg.localtname);

    if (stmt_select.execute() != 0) {
        // 执行查询语句，获取需要同步记录的键值
        logfile.write("stmt_select.execute() failed.\n%s\n%s\n", stmt_select.sql(), stmt_select.message());
        return false;
    }

    while (true) {
        // 获取需要同步数据的结果集
        if (stmt_select.next() != 0) {
            break;
        }

        strcpy(keyvalues[ccount], remotekeyvalue);
        ++ccount;

        // 每 starg.maxcount 条记录执行一次同步操作
        if (ccount == starg.maxcount) {
            // 从本地表中删除记录
            if (stmt_delete.execute() != 0) {
                // 执行从本地表中删除记录的操作一般不会出错
                // 如果报错，就肯定是数据库的问题或同步的参数配置不正确，流程不必继续
                logfile.write("stmt_delete.execute() failed.\n%s\n%s\n", stmt_delete.sql(), stmt_delete.message());
                return false;
            }

            // 向本地表中插入记录
            if (stmt_insert.execute() != 0) {
                // 执行从本地表中插入记录的操作一般不会出错
                // 如果报错，就肯定是数据库的问题或同步的参数配置不正确，流程不必继续
                logfile.write("stmt_insert.execute() failed.\n%s\n%s\n", stmt_insert.sql(), stmt_insert.message());
                return false;
            }

            //logfile.write("sync %s to %s %d rows in %.2f sec.\n", starg.linktname, starg.localtname, ccount, timer.elapsed());

            connlocal.commit();     // 不要忘记提交事务

            ccount = 0;     // 记录从结果集中已获取记录的计数器

            memset(keyvalues, 0, sizeof(keyvalues));

            // 执行一次分配同步操作，更新进程心跳
            pactive.updateAtime();
        }
    }

    // 如果还有剩余的分批记录，即小于 starg.maxcount 没有同步掉，需要再执行一次同步操作
    if (ccount > 0) {
        // 从本地表中删除记录
        if (stmt_delete.execute() != 0) {
            // 执行从本地表中删除记录的操作一般不会出错
            // 如果报错，就肯定是数据库的问题或同步的参数配置不正确，流程不必继续
            logfile.write("stmt_delete.execute() failed.\n%s\n%s\n", stmt_delete.sql(), stmt_delete.message());
            return false;
        }

        // 向本地表中插入记录
        if (stmt_insert.execute() != 0) {
            // 执行从本地表中插入记录的操作一般不会出错
            // 如果报错，就肯定是数据库的问题或同步的参数配置不正确，流程不必继续
            logfile.write("stmt_insert.execute() failed.\n%s\n%s\n", stmt_insert.sql(), stmt_insert.message());
            return false;
        }

        //logfile.write("sync %s to %s %d rows in %.2f sec.\n", starg.linktname, starg.localtname, ccount, timer.elapsed());

        connlocal.commit();     // 不要忘记提交事务
    }

    logfile << stmt_select.rpc() << " rows in " << timer.elapsed() << " sec.\n";
    return true;
}


// 程序退出主函数
void EXIT(int sig) {
    logfile.write("program exit, sig = %d\n", sig);
    connlocal.disconnect();
    exit(0);
}
