#include "_ooci.h"   // 开发框架操作Oracle的头文件
using namespace idc;

typedef struct GIRLS {
    long id;
    char name[31];      // 比数据库中字符串的最长长度 + 1，存储字符串结束符
    double weight;
    char btime[20];
    char memo[301];
}girls;

void execute_static_sql(connection& conn, sqlstatement& stmt);
void execute_dynamic_sql(connection& conn, sqlstatement& stmt);

int main(int argc, char* argv[])
{
    // 创建数据库连接类的对象
    connection conn;

    // 登录数据库
    if (conn.connecttodb("scott/123", "Simplified Chinese_China.AL32UTF8") != 0) {
        printf("connect database failed.\n%s\n", conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    sqlstatement stmt(&conn);

    int minid = 11, maxid = 13;

    // 准备查询表的SQL语句，prepare()方法不需要判断返回值。
    stmt.prepare("select id,name,weight,to_char(btime,'yyyy-mm-dd hh24:mi:ss'),memo\
        from girls\
        where id>=:1 and id<=:2");

    // 为 SQL 语句绑定输入变量的地址
    stmt.bindin(1, minid);
    stmt.bindin(2, maxid);

    // 把查询语句的结果集与变量的地址绑定，bindout()方法不需要判断返回值
    GIRLS girl;
    stmt.bindout(1, girl.id);
    stmt.bindout(2, girl.name, 30);
    stmt.bindout(3, girl.weight);
    stmt.bindout(4, girl.btime, 19);
    stmt.bindout(5, girl.memo, 300);

    // 执行SQL语句，一定要判断返回值，0-成功，其它-失败
    if (stmt.execute() != 0) {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    // 本程序执行的是查询语句，执行stmt.execute()后，将会在数据库的缓冲区中产生一个结果集
    while (true) {
        // 结构体变量初始化
        memset(&girl, 0, sizeof(girl));

        // 从结果集中获取一条记录，一定要判断返回值，0-成功，1403-无记录，其它-失败
        // 在实际开发中，除了0和1403，其它的情况极少出现。
        if (stmt.next() != 0) break;

        // 把获取到的记录的值打印出来。
        printf("id=%ld, name=%s, weight=%.02f, btime=%s, memo=%s.\n", girl.id, girl.name, girl.weight, girl.btime, girl.memo);
    }

    printf("successfully select %ld records.\n", stmt.rpc());

    return 0;
}

void execute_static_sql(connection& conn, sqlstatement& stmt) {

}

void execute_dynamic_sql(connection& conn, sqlstatement& stmt) {

}