#include "_public.h"
#include "_ooci.h"
using namespace idc;

// 全国气象站点参数结构体
typedef struct StStCode {
    char provname[31];      // 省
    char obtid[10];         // 站号
    char cityname[31];      // 站名
    char latitude[11];      // 维度
    char longitude[11];     // 经度
    char height[11];        // 海拔高度
}StStCode;

clogfile logfile;       // 日志文件对象
connection conn;        // 数据库连接对象
Cpactive pactive;       // 进程心跳
list<StStCode> st_code_list;    // 存放全国气象站点参数的容器

bool loadStCode(const string& inifile);     // 把站点参数文件内容加载到 st_code_list 容器中

void EXIT(int sig);     // 程序退出的信号处理函数

int main(int argc, char* argv[]) {
    // 帮助文档
    if (argc != 5) {
        printf("\n");
        printf("Using: ./obtcode_to_db inifile connstr charset logfile\n");

        printf("Example: /CppIndustrialMeteorologyProject/tools/bin/procctl 120 "\
            "/CppIndustrialMeteorologyProject/idc/bin/obtcode_to_db "\
            "/CppIndustrialMeteorologyProject/idc/ini/stcode.ini "\
            "\"idc/idcpwd\" "\
            "\"Simplified Chinese_China.AL32UTF8\" "\
            "/log/idc/obtcode_to_db.log\n\n");

        printf("本程序用于把全国气象站点参数数据保存到数据库的 T_ZHOBTCODE表中, 如果站点不存在则插入, 站点已存在则更新;\n");
        printf("inifile: 全国气象站点参数文件名(全路径);\n");
        printf("connstr: 数据库连接参数: username/password@tnsname;\n");
        printf("charset: 数据库的字符集;\n");
        printf("logfile: 本程序运行的日志文件名;\n");
        printf("程序每 120 秒运行一次, 由procctl调度。\n\n");
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

    // 进程心跳，设置 10s，如果超过 10s，杀死该进程
    pactive.addProcInfo(10, "obtcode_to_db");

    // 2. 把全国气象站点参数文件加载到容器中
    if (loadStCode(argv[1]) == false) {
        EXIT(-1);
    }

    // 3. 连接数据库
    if (conn.connecttodb(argv[2], argv[3]) != 0) {
        logfile.write("connect database ( %s ) failed.\n%s\n", argv[2]);
        EXIT(-1);
    }

    logfile.write("connect database ( %s ) ok.\n", argv[2]);

    // 4. 准备插入和更新表的 SQL 语句
    StStCode st_code;

    sqlstatement stmt_insert(&conn);
    stmt_insert.prepare("\
        insert into T_ZHOBTCODE(obtid,cityname,provname,latitude,longitude,height,keyid) \
        values(:1,:2,:3,:4*100,:5*100,:6*10,SEQ_ZHOBTCODE.nextval)"
    );
    // 动态 sql 语句绑定变量地址
    stmt_insert.bindin(1, st_code.obtid, 5);
    stmt_insert.bindin(2, st_code.cityname, 30);
    stmt_insert.bindin(3, st_code.provname, 30);
    stmt_insert.bindin(4, st_code.latitude, 10);
    stmt_insert.bindin(5, st_code.longitude, 10);
    stmt_insert.bindin(6, st_code.height, 10);

    sqlstatement stmt_update(&conn);
    stmt_update.prepare("\
        update T_ZHOBTCODE\
        set cityname=:1,provname=:2,latitude=:3*100,longitude=:4*100,height=:5*10, uptime=sysdate\
        where obtid=:6\
    ");
    // 动态 sql 语句绑定变量地址
    stmt_update.bindin(1, st_code.cityname, 30);
    stmt_update.bindin(2, st_code.provname, 30);
    stmt_update.bindin(3, st_code.latitude, 10);
    stmt_update.bindin(4, st_code.longitude, 10);
    stmt_update.bindin(5, st_code.height, 10);
    stmt_update.bindin(6, st_code.obtid, 5);
    // 以上代码要仔细

    int insert_count = 0, update_count = 0;     // 插入记录数和更新记录数
    ctimer timer;   // 定义定时器，计算操作数据库消耗的时间
    // 5. 遍历容器，处理每个站点，向表中插入记录，如果记录已经存在，则更新表中的记录
    for (auto& tmp : st_code_list) {
        st_code = tmp;      // 用到了开发框架中的运算符重载

        // 执行插入的 SQL 语句
        if (stmt_insert.execute() != 0) {
            if (stmt_insert.rc() == 1) {
                // 错误代码为 ORA-0001 表示违反主键唯一约束，表示数据库中已经存在了该站点记录
                // 执行更新的 SQL 语句
                if (stmt_update.execute() != 0) {
                    // 更新语句执行出错，程序退出
                    logfile.write("stmt_update.execute() failed.\n%s\n%s\n", stmt_update.sql(), stmt_update.message());
                    EXIT(-1);
                }
                else {
                    // update success
                    ++update_count;
                }
            }
            else {
                // 其它错误，程序退出
                logfile.write("stmt_insert.execute() failed.\n%s\n%s\n", stmt_insert.sql(), stmt_insert.message());
                EXIT(-1);
            }
        }
        else {
            // insert success
            ++insert_count;
        }
    }

    // 把总记录数，插入记录数，更新记录数，消耗时长记录到日志中
    logfile.write("total records = %d, insert records = %d, update records = %d, execute total time = %.2lfs\n",
        st_code_list.size(), insert_count, update_count, timer.elapsed());
    // 6. 提交事务
    conn.commit();
    return 0;
}

// 把站点参数文件内容加载到 st_code_list 容器中
bool loadStCode(const string& inifile) {
    cifile ifile;   // 读取文件对象

    if (ifile.open(inifile) == false) {
        // 写入文件之前，cpp 字符串风格转换为 c 字符串风格，可以防止乱码
        logfile.write("ifile.open( %s ) failed.\n", inifile.c_str());
        return false;
    }

    string str_buffer;      // 存放从文件中读取的每一行
    ifile.readline(str_buffer);     // 站点参数文件第一行是标题，无效

    ccmdstr cmdstr;     // 用于拆分从参数文件中读取的行
    StStCode st_code;    // 站点参数结构体

    while (ifile.readline(str_buffer)) {
        memset(&st_code, 0, sizeof(StStCode));

        cmdstr.splittocmd(str_buffer, ",");

        if (cmdstr.cmdcount() != 6) {
            continue;   // 扔掉无效行
        }
        cmdstr.getvalue(0, st_code.provname, 30);       // 省
        st_code.provname[30] = '\0';    // 强制指定字符串的结束符，防止插入数据时出现 ORA-01480 错误
        cmdstr.getvalue(1, st_code.obtid, 10);          // 站点编号
        st_code.obtid[5] = '\0';
        cmdstr.getvalue(2, st_code.cityname, 30);       // 站点名称
        st_code.cityname[30] = '\0';
        cmdstr.getvalue(3, st_code.latitude, 10);       // 维度
        st_code.latitude[10] = '\0';
        cmdstr.getvalue(4, st_code.longitude, 10);      // 经度
        st_code.longitude[10] = '\0';
        cmdstr.getvalue(5, st_code.height, 10);         // 海拔高度
        st_code.height[10] = '\0';

        st_code_list.push_back(st_code);     // 把站点参数存入到 list 容器中
    }

    return true;
}

// 程序退出的信号处理函数
void EXIT(int sig) {
    logfile.write("程序退出, sig = %d\n\n", sig);

    // 可以不写，在析构函数中会回滚事务和断开与数据库的连接
    conn.rollback();
    conn.disconnect();
}

