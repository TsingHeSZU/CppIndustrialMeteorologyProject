/*
    程序功能，采用增量同步的方法同步 Oracle 数据库的远程表
*/
#include "_tools.h"

// 程序运行参数的结构体
typedef struct StArg {
    char localconnstr[101];     // 本地数据库的连接参数
    char charset[51];           // 数据库的字符集
    char linktname[31];         // dblink 指向的远程表名，如T_ZHOBTCODE1@local
    char localtname[31];        // 本地表名
    char remotecols[1001];      // 远程表的字段列表
    char localcols[1001];       // 本地表的字段列表
    char where[1001];           // 同步数据的条件
    char remoteconnstr[101];    // 远程数据库的连接参数
    char remotetname[31];       // 远程表名
    char remotekeycol[31];      // 远程表的自增字段名
    char localkeycol[31];       // 本地表的自增字段名
    int maxcount;               // 执行一次增量同步操作的记录数
    int timetvl;                // 同步时间间隔，单位：秒，取值1-30
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

// 从本地表 starg.localtname 获取自增字段的最大值，存放在 maxkeyvalue 全局变量中
long maxkeyvalue = 0;

// 显示程序的帮助
void Help(char* argv[]);

// 把xml解析到参数starg结构中
bool xmlToArg(const char* strxmlbuffer);

// 业务处理主函数
bool syncIncrease(bool bcontinue);

bool loadMaxKey();

// 程序退出
void EXIT(int sig);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        Help(argv);
        return -1;
    }

    // 关闭全部的信号和输入输出，处理程序退出的信号
    //closeioandsignal(true); 
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

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
    pactive.addProcInfo(starg.timeout, starg.pname);

    // 连接本地数据库
    if (connlocal.connecttodb(starg.localconnstr, starg.charset) != 0) {
        logfile.write("connect local database(%s) failed.\n%s\n", starg.localconnstr, connlocal.message());
        EXIT(-1);
    }

    logfile.write("connect local database(%s) ok.\n", starg.localconnstr);

    // 连接远程数据库
    if (connremote.connecttodb(starg.remoteconnstr, starg.charset) != 0) {
        logfile.write("connect remote database(%s) failed.\n%s\n", starg.remoteconnstr, connremote.message());
        EXIT(-1);
    }

    logfile.write("connect remote database(%s) ok.\n", starg.remoteconnstr);

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

    // 为了保证数据同步的及时性，增量同步模块常驻内存，每隔 starg.timetvl 秒执行一次同步任务

    // true-本次同步了数据，false-本次没有同步数据
    bool bcontinue;

    while (true) {
        if (syncIncrease(bcontinue) == false) {
            EXIT(-1);
        }

        // 如果本次没有同步到数据，就休眠
        if (bcontinue == false) {
            sleep(starg.timetvl);
        }

        pactive.updateAtime();
    }
}

// 程序的帮助
void Help(char* argv[]) {
    printf("Using: /CppIndustrialMeteorologyProject/tools/bin/syncincrease logfilename xmlbuffer\n");
    printf("Sample:\n");
    printf("\n把 T_ZHOBTMIND1@local 表中全部的记录增量同步到 T_ZHOBTMIND2 中;\n");
    printf("/CppIndustrialMeteorologyProject/tools/bin/procctl 10 "\
        "/CppIndustrialMeteorologyProject/tools/bin/syncincrease "\
        "/log/idc/syncincrease_ZHOBTMIND2.log "\
        "\"<localconnstr>idc1/idcpwd1</localconnstr>"\
        "<remoteconnstr>idc/idcpwd</remoteconnstr>"\
        "<charset>Simplified Chinese_China.AL32UTF8</charset>"\
        "<remotetname>T_ZHOBTMIND1</remotetname>"\
        "<linktname>T_ZHOBTMIND1@local</linktname>"\
        "<localtname>T_ZHOBTMIND2</localtname>"\
        "<remotecols>obtid,ddatetime,t,p,rh,wd,ws,r,vis,uptime,keyid</remotecols>"\
        "<localcols>stid,ddatetime,t,p,rh,wd,ws,r,vis,uptime,recid</localcols>"\
        "<remotekeycol>keyid</remotekeycol>"\
        "<localkeycol>recid</localkeycol>"\
        "<maxcount>300</maxcount>"\
        "<timetvl>2</timetvl>"\
        "<timeout>50</timeout>"\
        "<pname>syncincrease_ZHOBTMIND2</pname>\"\n");

    printf("\n把 T_ZHOBTMIND1@local 表中满足 \"and obtid like '57%%'\" 的记录增量同步到 T_ZHOBTMIND3 中;\n");
    printf("/CppIndustrialMeteorologyProject/tools/bin/procctl 10 "\
        "/CppIndustrialMeteorologyProject/tools/bin/syncincrease "
        "/log/idc/syncincrease_ZHOBTMIND3.log "\
        "\"<localconnstr>idc1/idcpwd1</localconnstr>"\
        "<remoteconnstr>idc/idcpwd</remoteconnstr>"\
        "<charset>Simplified Chinese_China.AL32UTF8</charset>"\
        "<remotetname>T_ZHOBTMIND1</remotetname>"\
        "<linktname>T_ZHOBTMIND1@local</linktname>"\
        "<localtname>T_ZHOBTMIND3</localtname>"\
        "<remotecols>obtid,ddatetime,t,p,rh,wd,ws,r,vis,uptime,keyid</remotecols>"\
        "<localcols>stid,ddatetime,t,p,rh,wd,ws,r,vis,uptime,recid</localcols>"\
        "<where>and obtid like '54%%%%'</where>"\
        "<remotekeycol>keyid</remotekeycol>"\
        "<localkeycol>recid</localkeycol>"\
        "<maxcount>300</maxcount>"\
        "<timetvl>2</timetvl>"\
        "<timeout>30</timeout>"\
        "<pname>syncincrease_ZHOBTMIND3</pname>\"\n");

    printf("\n本程序是共享平台的公共功能模块, 采用增量的方法同步Oracle数据库之间的表;\n");
    printf("logfilename: 本程序运行的日志文件;\n");
    printf("xmlbuffer: 本程序运行的参数, 用 xml 表示, 具体如下：\n");
    printf("localconnst: 本地数据库的连接参数, 格式: username/passwd@tnsname;\n");
    printf("charset: 数据库的字符集, 这个参数要与远程数据库保持一致, 否则会出现中文乱码的情况;\n");
    printf("linktname: 远程表名, 在 remotetname 参数后加 @dblink;\n");
    printf("localtname: 本地表名;\n");
    printf("remotecols: 远程表的字段列表, 用于填充在 select 和 from 之间, 所以, remotecols可以是真实的字段, "\
        "也可以是函数的返回值或者运算结果; 如果本参数为空, 就用 localtname 表的字段列表填充;\n");
    printf("localcols: 本地表的字段列表, 与 remotecols 不同, 它必须是真实存在的字段; 如果本参数为空, "\
        "就用 localtname 表的字段列表填充;\n");
    printf("where: 同步数据的条件, 填充在select starg.remotekeycol from remotetname where starg.remotekeycol>:1之后, "\
        "注意, 不要加 where 关键字, 但是, 需要加 and 关键字, 本参数可以为空;\n");
    printf("remoteconnstr: 远程数据库的连接参数, 格式与 localconnstr 相同;\n");
    printf("remotetname: 远程表名, 表名后面不要加 dblink;\n");
    printf("remotekeycol: 远程表的自增字段名;\n");
    printf("localkeycol: 本地表的自增字段名;\n");
    printf("maxcount: 每批操作的记录数, 建议在 100-500 之间;\n");
    printf("timetvl: 执行同步任务的时间间隔, 单位：秒, 取值 1-30;\n");
    printf("timeout: 本程序的超时时间, 单位：秒, 视数据量的大小而定, 建议设置 30 以上;\n");
    printf("pname: 本程序运行时的进程名, 尽可能采用易懂的, 与其它进程不同的名称, 方便故障排查;\n\n");
}

// 把 xml 解析到参数 starg 结构中
bool xmlToArg(const char* strxmlbuffer) {
    memset(&starg, 0, sizeof(StArg));

    // 本地数据库的连接参数
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

    // 远程表的字段列表
    getxmlbuffer(strxmlbuffer, "remotecols", starg.remotecols, 1000);

    // 本地表的字段列表
    getxmlbuffer(strxmlbuffer, "localcols", starg.localcols, 1000);

    // 同步数据的条件，即 select 语句的 where 部分
    getxmlbuffer(strxmlbuffer, "where", starg.where, 1000);

    // 远程数据库的连接参数
    getxmlbuffer(strxmlbuffer, "remoteconnstr", starg.remoteconnstr, 100);
    if (strlen(starg.remoteconnstr) == 0) {
        logfile.write("remoteconnstr is null.\n");
        return false;
    }

    // 远程表名
    getxmlbuffer(strxmlbuffer, "remotetname", starg.remotetname, 30);
    if (strlen(starg.remotetname) == 0) {
        logfile.write("remotetname is null.\n");
        return false;
    }

    // 远程表的自增字段名
    getxmlbuffer(strxmlbuffer, "remotekeycol", starg.remotekeycol, 30);
    if (strlen(starg.remotekeycol) == 0) {
        logfile.write("remotekeycol is null.\n");
        return false;
    }

    // 本地表的自增字段名
    getxmlbuffer(strxmlbuffer, "localkeycol", starg.localkeycol, 30);
    if (strlen(starg.localkeycol) == 0) {
        logfile.write("localkeycol is null.\n");
        return false;
    }

    // 每批执行一次同步操作的记录数
    getxmlbuffer(strxmlbuffer, "maxcount", starg.maxcount);
    if (starg.maxcount == 0) {
        logfile.write("maxcount is null.\n");
        return false;
    }

    // 执行同步的时间间隔，单位：秒，取值1-30
    getxmlbuffer(strxmlbuffer, "timetvl", starg.timetvl);
    if (starg.timetvl <= 0) {
        logfile.write("timetvl is null.\n");
        return false;
    }

    if (starg.timetvl > 30) {
        starg.timetvl = 30;
    }
    // 程序的超时时间，单位：秒，视数据量的大小而定，建议设置 30 以上
    getxmlbuffer(strxmlbuffer, "timeout", starg.timeout);
    if (starg.timeout == 0) {
        logfile.write("timeout is null.\n");
        return false;
    }

    // 以下处理 timetvl 和 timeout 的方法虽然有点随意，但也问题不大，不让程序超时就可以了
    if (starg.timeout < starg.timetvl + 10) {
        starg.timeout = starg.timetvl + 10;
    }

    // 进程名
    getxmlbuffer(strxmlbuffer, "pname", starg.pname, 50);
    if (strlen(starg.pname) == 0) {
        logfile.write("pname is null.\n");
        return false;
    }

    return true;
}

// 业务处理主函数
bool syncIncrease(bool bcontinue) {
    ctimer timer;
    bcontinue = false;

    // 从本地表 starg.localtname 获取自增字段的最大值，存放在 maxkeyvalue 中
    if (loadMaxKey() == false) {
        return false;
    }

    // 从本地表查找自增字段的值大于 maxkeyvalue 的记录
    char remoterowidvalue[21];    // 从远程表查到的需要同步记录的自增字段的值
    sqlstatement stmt_select(&connremote);
    stmt_select.prepare("select rowid from %s where %s>:1 %s order by %s", starg.remotetname,
        starg.remotekeycol, starg.where, starg.remotekeycol);

    stmt_select.bindin(1, maxkeyvalue);
    stmt_select.bindout(1, remoterowidvalue, 20);

    // 拼接绑定同步 sql 语句参数的字符串（:1,:2,:3,...,:starg.maxcount）
    string bindstr;
    for (int i = 0;i < starg.maxcount;++i) {
        bindstr = bindstr + sformat(":%lu,", i + 1);
    }

    deleterchr(bindstr, ',');

    // 存放自增字段的值
    char rowidvalues[starg.maxcount][21];

    // 准备插入本地表数据的 sql 语句，一次插入 starg.maxcount 条记录
    sqlstatement stmt_insert(&connlocal);
    stmt_insert.prepare("insert into %s(%s) select %s from %s where rowid in (%s)", starg.localtname,
        starg.localcols, starg.remotecols, starg.linktname, bindstr.c_str());

    for (int i = 0;i < starg.maxcount;++i) {
        stmt_insert.bindin(i + 1, rowidvalues[i]);
    }

    // 记录从结果集中获取记录的计数器
    int ccount = 0;

    memset(rowidvalues, 0, sizeof(rowidvalues));
    if (stmt_select.execute() != 0) {
        logfile.write("stmt_select.execute() failed.\n%s\n%s\n", stmt_select.sql(), stmt_select.message());
        return false;
    }

    while (true) {
        // 获取需要同步数据的结果集
        if (stmt_select.next() != 0) {
            break;
        }
        strcpy(rowidvalues[ccount], remoterowidvalue);
        ++ccount;

        // 每 starg.maxcount 条记录执行一次同步
        if (ccount == starg.maxcount) {
            // 向本地表中插入记录
            if (stmt_insert.execute() != 0) {
                logfile.write("stmt_insert.execute() failed.\n%s\n%s\n", stmt_insert.sql(), stmt_insert.message());
                return false;
            }
            connlocal.commit(); // 提交事务
            ccount = 0;         // 记录从结果集中已获取记录的计数器
            memset(rowidvalues, 0, sizeof(rowidvalues));

            pactive.updateAtime();
        }
    }

    // 说明还有数据没有增量同步
    if (ccount > 0) {
        // 向本地表中插入记录
        if (stmt_insert.execute() != 0) {
            logfile.write("stmt_insert.execute() failed.\n%s\n%s\n", stmt_insert.sql(), stmt_insert.message());
            return false;
        }
        connlocal.commit(); // 提交事务

        pactive.updateAtime();
    }

    if (stmt_select.rpc() > 0) {
        logfile.write("sync %s to %s(%d rows) in %.2f sec.\n", starg.linktname, starg.localtname, stmt_select.rpc(), timer.elapsed());
        bcontinue = true;
    }

    return true;
}

bool loadMaxKey() {
    maxkeyvalue = 0;
    sqlstatement stmt(&connlocal);
    stmt.prepare("select max(%s) from %s", starg.localkeycol, starg.localtname);
    stmt.bindout(1, maxkeyvalue);

    if (stmt.execute() != 0) {
        logfile.write("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return false;
    }

    // 获取查询的第一个结果集
    stmt.next();

    // logfile.write("maxkeyvalue = %ld.\n", maxkeyvalue);

    return true;
}

// 程序退出主函数
void EXIT(int sig) {
    logfile.write("program exit, sig = %d\n", sig);
    connlocal.disconnect();
    connremote.disconnect();
    exit(0);
}
