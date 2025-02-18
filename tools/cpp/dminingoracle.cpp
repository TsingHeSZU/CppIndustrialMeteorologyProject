#include "_public.h"
#include "_ooci.h"
using namespace idc;

// 程序运行参数的结构体
typedef struct StArg {
    char connstr[101];          // 数据库的连接参数
    char charset[51];           // 数据库的字符集
    char selectsql[1024];       // 从数据源数据库抽取数据的SQL语句
    char fieldstr[501];         // 抽取数据的 SQL 语句输出结果集字段名，字段名之间用逗号分隔
    char fieldlen[501];         // 抽取数据的 SQL 语句输出结果集字段的长度，用逗号分隔
    char bfilename[31];         // 输出 xml 文件的前缀
    char efilename[31];         // 输出 xml 文件的后缀
    char outpath[256];          // 输出 xml 文件存放的目录
    int maxcount;               // 输出 xml 文件最大记录数，0表示无限制, xml 文件将用于入库，如果文件太大，数据库会产生大事务
    char starttime[52];         // 程序运行的时间区间
    char incfield[31];          // 递增字段名
    char incfilename[256];      // 已抽取数据的递增字段最大值存放的文件
    char connstr1[101];         // 已抽取数据的递增字段最大值存放的数据库的连接参数
    int timeout;                // 进程心跳的超时时间
    char pname[51];             // 进程名，建议用 "dminingoracle_后缀" 的方式   
}StArg;

Cpactive pactive;       // 进程心跳
clogfile logfile;       // 日志文件
StArg starg;            // 程序运行参数
ccmdstr all_fieldstr;      // 结果集字段名数组
ccmdstr all_fieldlen;       // 结果集字段的长度数组
connection conn;        // 数据源数据库


void Help();                    // 程序帮助文档

bool xmlToArg(const char* strxmlbuffer);    // 把 xml 解析到参数 starg 结构体变量中
bool inStartTime();             // 判断当前时间是否在程序运行的时间区间内
bool dminingOracle();           // 数据抽取主函数

// 增量抽取数据库的内容

// 1. 从文件或者数据库中获取上次已抽取数据的增量字段的最大值（如果是第一次执行抽取任务，增量字段最大值为 0）
long imaxincvalue;      // 增量抽取递增字段的最大值
bool readIncreaseField();            // 从数据库表中或者 starg.incfilename 文件中加载上次已抽取数据的最大值

// 2. 在数据抽取的 sql 语句执行前，绑定输入变量（已抽取数据增量字段的最大值）

// 3. 获取结果集的时候，要把递增字段的最大值保存在临时变量中
int incfieldpos = -1;   // 递增字段在结果集数组中的位置

// 4. 抽取完数据之后，把临时变量中的增量字段的最大值保存在文件或数据库中
bool writeInceaseField();   // 把增量抽取数据的递增字段最大值写入数据库表或 starg.incfilename 文件中

void EXIT(int sig);             // 程序退出和信号2, 15 的处理函数



int main(int argc, char* argv[]) {
    if (argc != 3) {
        Help();
        return -1;
    }

    // 关闭全部的信号和输入输出
    //closeioandsignal(true);
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    // 打开日志文件
    if (logfile.open(argv[1]) == false) {
        printf("open logfile(%s) failed.\n", argv[1]);
        return -1;
    }

    // 解析 xml 格式的字符串，得到程序运行的参数
    if (xmlToArg(argv[2]) == false) {
        logfile.write("parse xml string (%s) failed.\n", argv[2]);
        EXIT(-1);
    }

    // 判断当前时间是否在程序运行的时间区间内
    if (inStartTime() == false) {
        return 0;
    }

    // 进程心跳信息加入共享内存
    pactive.addProcInfo(starg.timeout, starg.pname);

    // 数据库连接
    if (conn.connecttodb(starg.connstr, starg.charset) != 0) {
        logfile.write("connect database(%s) failed.\n%s\n", starg.connstr, conn.message());
        EXIT(-1);
    }
    logfile.write("connect database(%s) ok.\n", starg.connstr);

    // 从数据库表中或 starg.incfilename 文件中获取已抽取数据的最大值，用于增量抽取
    if (readIncreaseField() == false) {
        EXIT(-1);
    }

    // 数据抽取主函数
    dminingOracle();

    return 0;
}


// 程序帮助文档
void Help() {
    printf("Using: /CppIndustrialMeteorologyProject/tools/bin/dminingoracle logfilename xmlbuffer\n\n");

    // sql 语句的 5%%%% 中，因为 % 在 cpp 程序中是特殊字符，%% 表示 %，又因为 stmt.prepare() 接收的字符串也会处理特殊字符，所以最后是 5% 
    printf("Sample:\n/CppIndustrialMeteorologyProject/tools/bin/procctl 3600 "\
        "/CppIndustrialMeteorologyProject/tools/bin/dminingoracle /log/idc/dminingoracle_ZHOBTCODE.log "\
        "\"<connstr>idc/idcpwd</connstr>"\
        "<charset>Simplified Chinese_China.AL32UTF8</charset>"\
        "<selectsql>select obtid,cityname,provname,latitude,longitude,height from T_ZHOBTCODE where obtid like '5%%%%'</selectsql>"\
        "<fieldstr>obtid,cityname,provname,latitude,longitude,height</fieldstr><fieldlen>5,30,30,10,10,10</fieldlen>"\
        "<bfilename>ZHOBTCODE</bfilename>"\
        "<efilename>togxpt</efilename>"\
        "<outpath>/idcdata/dmindata</outpath>"\
        "<timeout>30</timeout>"\
        "<pname>dminingoracle_ZHOBTCODE</pname>\"\n\n");
    printf("/CppIndustrialMeteorologyProject/tools/bin/procctl 30 "\
        "/CppIndustrialMeteorologyProject/tools/bin/dminingoracle /log/idc/dminingoracle_ZHOBTMIND.log "\
        "\"<connstr>idc/idcpwd</connstr>"\
        "<charset>Simplified Chinese_China.AL32UTF8</charset>"\
        "<selectsql>select obtid,to_char(ddatetime,'yyyymmddhh24miss'),t,p,rh,wd,ws,r,vis,keyid from T_ZHOBTMIND "\
        "where keyid>:1 and obtid like '5%%%%'</selectsql>"\
        "<fieldstr>obtid,ddatetime,t,p,rh,wd,ws,r,vis,keyid</fieldstr>"\
        "<fieldlen>5,19,8,8,8,8,8,8,8,15</fieldlen>"\
        "<bfilename>ZHOBTMIND</bfilename>"\
        "<efilename>togxpt</efilename>"\
        "<outpath>/idcdata/dmindata</outpath>"\
        "<starttime></starttime>"\
        "<incfield>keyid</incfield>"\
        "<incfilename>/idcdata/dmining/dminingoracle_ZHOBTMIND_togxpt.keyid</incfilename>"\
        "<timeout>30</timeout>"\
        "<pname>dminingoracle_ZHOBTMIND_togxpt</pname>"\
        "<maxcount>1000</maxcount>"\
        "<connstr1>scott/123</connstr1>\"\n\n");

    printf("本程序是共享平台的公共功能模块, 用于从Oracle数据库源表抽取数据, 生成xml文件; \n");
    printf("logfilename: 本程序运行的日志文件; \n");
    printf("xmlbuffer: 本程序运行的参数, 用 xml 表示, 具体如下:\n\n");
    printf("connstr: 数据源数据库的连接参数, 格式: username/passwd@tnsname;\n");
    printf("charset: 数据库的字符集, 这个参数要与数据源数据库保持一致, 否则会出现中文乱码的情况;\n");
    printf("selectsql: 从数据源数据库抽取数据的SQL语句, 如果是增量抽取, 一定要用递增字段作为查询条件, 如 where keyid>:1;\n");
    printf("fieldstr: 抽取数据的 SQL 语句输出结果集的字段名列表, 中间用逗号分隔, 将作为 xml 文件的字段名;\n");
    printf("fieldlen: 抽取数据的 SQL 语句输出结果集字段的长度列表, 中间用逗号分隔, fieldstr 与 fieldlen 的字段必须一一对应;\n");
    printf("outpath: 输出 xml 文件存放的目录;\n");
    printf("bfilename: 输出 xml 文件的前缀;\n");
    printf("efilename: 输出 xml 文件的后缀;\n");
    printf("maxcount: 输出 xml 文件的最大记录数, 缺省是0, 表示无限制, 如果本参数取值为0, 注意适当加大 timeout 的取值, 防止程序超时;\n");
    printf("starttime: 程序运行的时间区间, 例如 [02, 13] 表示: 如果程序启动时, 在 02 时和 13 时区间内则运行, 其它时间不运行, "\
        "如果 starttime 为空, 表示不启用, 只要本程序启动, 就会执行数据抽取任务, 为了减少数据源数据库压力, "\
        "抽取数据的时候, 如果对时效性没有要求, 一般在数据源数据库空闲的时候时进行;\n");
    printf("incfield: 递增字段名, 它必须是 fieldstr 中的字段名, 并且只能是整型, 一般为自增字段, "\
        "如果incfield为空, 表示不采用增量抽取的方案;");
    printf("incfilename: 已抽取数据的递增字段最大值存放的文件, 如果该文件丢失, 将重新抽取全部的数据;\n");
    printf("connstr1: 已抽取数据的递增字段最大值存放的数据库的连接参数, connstr1 和 incfilename 二选一, connstr1 优先;");
    printf("timeout: 本程序的超时时间, 单位: 秒;\n");
    printf("pname: 进程名, 尽可能采用易懂的、与其它进程不同的名称, 方便故障排查。\n\n");
}

// 把 xml 解析到参数 starg 结构体变量中
bool xmlToArg(const char* strxmlbuffer) {
    memset(&starg, 0, sizeof(StArg));

    // 数据源数据库的连接参数
    getxmlbuffer(strxmlbuffer, "connstr", starg.connstr, 100);
    if (strlen(starg.connstr) == 0) {
        logfile.write("connstr is null.\n");
        return false;
    }

    // 数据库的字符集
    getxmlbuffer(strxmlbuffer, "charset", starg.charset, 50);
    if (strlen(starg.charset) == 0) {
        logfile.write("charset is null.\n");
        return false;
    }

    // 从数据源数据库抽取数据的 SQL 语句
    getxmlbuffer(strxmlbuffer, "selectsql", starg.selectsql, 1000);
    if (strlen(starg.selectsql) == 0) {
        logfile.write("selectsql is null.\n");
        return false;
    }

    // 结果集字段名列表
    getxmlbuffer(strxmlbuffer, "fieldstr", starg.fieldstr, 500);
    if (strlen(starg.fieldstr) == 0) {
        logfile.write("fieldstr is null.\n");
        return false;
    }

    // 结果集字段长度列表
    getxmlbuffer(strxmlbuffer, "fieldlen", starg.fieldlen, 500);
    if (strlen(starg.fieldlen) == 0) {
        logfile.write("fieldlen is null.\n");
        return false;
    }

    // 输出 xml 文件存放的目录
    getxmlbuffer(strxmlbuffer, "bfilename", starg.bfilename, 30);
    if (strlen(starg.bfilename) == 0) {
        logfile.write("bfilename is null.\n");
        return false;
    }

    // 输出 xml 文件的前缀
    getxmlbuffer(strxmlbuffer, "efilename", starg.efilename, 30);
    if (strlen(starg.efilename) == 0) {
        logfile.write("efilename is null.\n");
        return false;
    }

    // 输出xml文件的后缀
    getxmlbuffer(strxmlbuffer, "outpath", starg.outpath, 255);
    if (strlen(starg.outpath) == 0) {
        logfile.write("outpath is null.\n");
        return false;
    }

    // 输出xml文件的最大记录数，可选参数，如果没有，默认赋值为 0
    if (getxmlbuffer(strxmlbuffer, "maxcount", starg.maxcount) == false) {
        starg.maxcount = 0;
    }

    // 程序运行的时间区间，可选参数
    getxmlbuffer(strxmlbuffer, "starttime", starg.starttime, 50);

    // 递增字段名，可选参数
    getxmlbuffer(strxmlbuffer, "incfield", starg.incfield, 30);

    // 已抽取数据的递增字段最大值存放的文件，可选参数
    getxmlbuffer(strxmlbuffer, "incfilename", starg.incfilename, 255);

    // 已抽取数据的递增字段最大值存放的数据库的连接参数，可选参数
    getxmlbuffer(strxmlbuffer, "connstr1", starg.connstr1, 100);

    // 进程心跳的超时时间
    getxmlbuffer(strxmlbuffer, "timeout", starg.timeout);
    if (starg.timeout == 0) {
        logfile.write("timeout is null.\n");
        return false;
    }

    // 进程名
    getxmlbuffer(strxmlbuffer, "pname", starg.pname, 50);
    if (strlen(starg.pname) == 0) {
        logfile.write("pname is null.\n");
        return false;
    }

    // 拆分 starg.fieldstr 到 all_fieldstr 中
    all_fieldstr.splittocmd(starg.fieldstr, ",");

    // 拆分 starg.fieldlen 到 fieldlen 中
    all_fieldlen.splittocmd(starg.fieldlen, ",");

    // 判断 all_fieldstr 和 fieldlen 两个数组的大小是否相同
    if (all_fieldlen.size() != all_fieldstr.size()) {
        logfile.write("fieldstr 和 fieldlen 的元素个数不一致。\n");
        return false;
    }

    // 如果是增量抽取，incfilename 和 connstr1 必二选一
    if (strlen(starg.incfield) > 0) {
        if ((strlen(starg.incfilename) == 0) && (strlen(starg.connstr1) == 0)) {
            logfile.write("如果是增量抽取, incfilename 和 connstr1 必二选一, 不能都为空。\n");
            return false;
        }
    }

    return true;
}

// 判断当前时间是否在程序运行的时间区间内
bool inStartTime() {
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

// 数据抽取主函数
bool dminingOracle() {
    // 1. 准备抽取数据的 SQL 语句
    sqlstatement stmt(&conn);
    stmt.prepare(starg.selectsql);
    // 2. 绑定结果集的变量
    string strfieldvalue[all_fieldstr.size()];
    for (int i = 1;i <= all_fieldstr.size();++i) {
        stmt.bindout(i, strfieldvalue[i - 1], stoi(all_fieldlen[i - 1]));
    }

    // 如果是增量抽取，绑定输入参数（已抽取数据的递增字段的最大值）
    if (strlen(starg.incfield) != 0) {
        stmt.bindin(1, imaxincvalue);
    }

    // 3. 执行抽取数据的 SQL 语句
    if (stmt.execute() != 0) {
        logfile.write("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return false;
    }

    pactive.updateAtime();      // 执行抽取数据的 SQL 语句后，更新进程心跳信息

    // 4. 获取结果集中的记录，写入 xml 文件
    string strxmlfilename;      // 输出的 xml 文件名，例如：ZHOBTCODE_20250218094835_togxpt_1.xml
    cofile ofile;               // 用于向 xml 文件中写入数据
    int iseq = 1;               // 输出 xml 文件的序号 

    while (true) {
        // 结果集为空，不会创建 xml 文件
        if (stmt.next() != 0) {
            break;
        }

        // 打开 xml 文件，需要全路径
        if (ofile.isopen() == false) {
            sformat(strxmlfilename, "%s/%s_%s_%s_%d.xml", starg.outpath, starg.bfilename,
                ltime1("yyyymmddhh24miss").c_str(), starg.efilename, iseq++);
            if (ofile.open(strxmlfilename) == false) {
                logfile.write("ofile.open(%s) failed.\n", strxmlfilename.c_str());
                return false;
            }
            ofile.writeline("<data>\n");    // 写入数据集开始的标签
        }

        // 把结果集中每个字段的值写入 xml 文件
        for (int i = 0;i < all_fieldstr.size();++i) {
            ofile.writeline("<%s>%s</%s>", all_fieldstr[i].c_str(), strfieldvalue[i].c_str(), all_fieldstr[i].c_str());
        }
        // 写入每行的结束标志
        ofile.writeline("<endl/>\n");
        //printf("stmt.rpc() = %ld.\n", stmt.rpc());

        // 如果记录数达到了 max_count 行，就关闭当前文件，stmt.rpc() 获取 select 语句查询当前数据集的第几行
        if ((starg.maxcount > 0) && (stmt.rpc() % starg.maxcount == 0)) {
            ofile.writeline("</data>\n");   // 写入文件的结束标志
            if (ofile.closeandrename() == false) {
                // 关闭文件，把临时文件名改为正式文件名
                logfile.write("ofile.closeandrename(%s) failed.\n", strxmlfilename.c_str());
                return false;
            }
            logfile.write("create xml file %s(%d) ok.\n", strxmlfilename.c_str(), starg.maxcount);

            pactive.updateAtime();      // 生成一个 xml 文件后，更新进程心跳
        }

        // 更新增量抽取，递增字段的最大值
        if (strlen(starg.incfield) != 0 && imaxincvalue < stoi(strfieldvalue[incfieldpos])) {
            imaxincvalue = stoi(strfieldvalue[incfieldpos]);
        }

    }

    // 5. 如果 maxcount==0 或者向 xml 文件中写入的记录数不足 maxcount，关闭文件
    if (ofile.isopen() == true) {
        ofile.writeline("</data>\n");       // 不要忘记数据集结束的标签
        if (ofile.closeandrename() == false) {
            logfile.write("ofile.closeandrename(%s) failed,\n", strxmlfilename.c_str());
            return false;
        }
        if (starg.maxcount == 0) {
            logfile.write("create xml file %s(%d) ok.\n", strxmlfilename.c_str(), stmt.rpc());
        }
        else {
            logfile.write("create xml file %s(%d) ok.\n", strxmlfilename.c_str(), stmt.rpc() % starg.maxcount);
        }
    }

    // 把已抽取数据的递增字段最大值写入到数据库表或者 starg.incfilename 文件中
    if (stmt.rpc() > 0) {
        writeInceaseField();
    }
    return true;
}

// 从数据库表中或者 starg.incfilename 文件中加载上次已抽取数据的最大值
bool readIncreaseField() {
    imaxincvalue = 0;   // 初始化递增字段的最大值

    // 如果 starg.incfield 参数为空，表示不是增量抽取
    if (strlen(starg.incfield) == 0) {
        return true;
    }

    // 查找递增字段在结果集中的位置
    for (int i = 0;i < all_fieldstr.size();++i) {
        if (all_fieldstr[i] == starg.incfield) {
            incfieldpos = i;
            break;
        }
    }
    if (incfieldpos == -1) {
        // 没有找到递增字段在结果集中的位置，程序运行参数不正确
        logfile.write("increase field(%s) is not at fields(%s).\n", starg.incfield, starg.fieldstr);
        return false;
    }

    if (strlen(starg.connstr1) != 0) {
        // 从数据库表中加载递增字段的最大值
        connection conn1;
        if (conn1.connecttodb(starg.connstr1, starg.charset) != 0) {
            logfile.write("connect database(%s) failed.\n%s\n", starg.connstr1, conn1.message());
            return false;
        }

        sqlstatement stmt(&conn1);
        stmt.prepare("select maxincvalue from T_MAXINCVALUE where pname=:1");
        stmt.bindin(1, starg.pname);
        stmt.bindout(1, imaxincvalue);
        if (stmt.execute() != 0) {
            // sql 语句执行失败，可能是第一次运行程序，存储递增字段的表不存在
            if (stmt.rc() == 942) {
                logfile.write("first execute program, stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
            }
            else {
                logfile.write("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
                return false;
            }
        }
        stmt.next();    // 获取查询结果集中的第一行，此时 stmt.rpc() = 1
    }
    else {
        // 从文件中加载递增字段的最大值
        cifile ifile;

        // 如果打开 starg.incfilename 文件失败，表示是第一次运行程序，不必返回 false
        // 也可能是文件丢失了，只能重新增量抽取全部数据
        if (ifile.open(starg.incfilename) == false) {
            return true;
        }

        // 从文件中读取已抽取数据的最大值
        string strtmp;
        ifile.readline(strtmp);
        imaxincvalue = stoi(strtmp);
    }
    logfile.write("the location of last obtian data from oralce is (%s = %ld).\n", starg.incfield, imaxincvalue);

    return true;
}

// 把增量抽取数据的递增字段最大值写入数据库表或 starg.incfilename 文件中
bool writeInceaseField() {
    // 如果递增字段为空，表示不是增量抽取
    if (strlen(starg.incfield) == 0) {
        return true;
    }

    if (strlen(starg.connstr1) != 0) {
        // 把递增字段的最大值写入数据库表
        connection conn1;
        if (conn1.connecttodb(starg.connstr1, starg.charset) != 0) {
            logfile.write("connect database(%s) failed.\n%s\n", starg.connstr1, conn1.message());
            return false;
        }
        sqlstatement stmt(&conn1);
        stmt.prepare("update T_MAXINCVALUE set maxincvalue=:1 where pname=:2");
        stmt.bindin(1, imaxincvalue);
        stmt.bindin(2, starg.pname);

        if (stmt.execute() != 0) {
            // 如果表不存在，将返回 ORA-00942 错误
            if (stmt.rc() == 942) {
                // 如果表不存在，创建表并且插入数据
                // connection 类也可以执行简单的 sql 语句，这里不同 stmt 执行是因为 stmt 绑定了输入参数
                conn1.execute("create table T_MAXINCVALUE(pname varchar2(50),maxincvalue number(15),primary key(pname))");
                conn1.execute("insert into T_MAXINCVALUE values('%s',%ld)", starg.pname, imaxincvalue);
                conn1.commit();     // 修改了表的数据不要忘记提交事务
                return true;
            }
            else {
                // 其它错误，sql 语句执行失败
                logfile.write("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
                return false;
            }
        }
        else {
            if (stmt.rpc() == 0) {
                // 如果记录不存在，就插入新记录
                conn1.execute("insert into T_MAXINCVALUE values('%s',%ld)", starg.pname, imaxincvalue);
            }
            conn1.commit();
        }
    }
    else {
        // 把递增字段最大值写入文件
        cofile ofile;
        if (ofile.open(starg.incfilename, false) == false) {
            logfile.write("ofile.open(%s) failed.\n", starg.incfilename);
            return false;
        }

        // 把已抽取数据的递增字段最大值写入文件
        ofile.writeline("%ld", imaxincvalue);
    }
    return true;
}

// 程序退出和信号2, 15 的处理函数
void EXIT(int sig) {
    logfile.write("program exit, sig = %d.\n", sig);
    exit(0);
}