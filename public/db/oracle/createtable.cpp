#include "_ooci.h"   // 开发框架操作 Oracle 的头文件
using namespace idc;

int main(int argc, char* argv[]) {
    connection conn;    // 创建数据库连接类对象

    // 连接数据库
    if (conn.connecttodb("scott/123", "Simplified Chinese_China.AL32UTF8") != 0) {
        printf("connect database failed.\n%d,%s\n", conn.rc(), conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    sqlstatement stmt;      // 定义操作 sql 语句的对象
    stmt.connect(&conn);    // 指定 stmt 对象使用的数据库连接

    // 准备 sql 语句
    stmt.prepare("\
        create table girls(\
            id    number(10),\
            name  varchar2(30),\
            weight   number(8,2),\
            btime date,\
            memo  varchar2(300),\
            pic   blob,\
            primary key (id)\
        )");

    // 执行 sql 语句，一定要判断返回值，0-成功，其它-失败
    // 失败代码在 stmt.m_cda.rc 中，失败描述在 stmt.m_cda.message 中
    if (stmt.execute() != 0) {
        printf("stmt.excute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    printf("create table girls ok.\n");

    // 退出数据库连接
    conn.disconnect();
    return 0;
}