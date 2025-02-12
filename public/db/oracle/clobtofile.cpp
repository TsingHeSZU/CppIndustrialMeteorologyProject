#include "_ooci.h"   // 开发框架操作Oracle的头文件
using namespace idc;

int main(int argc, char* argv[]) {
    // 创建数据库连接类的对象
    connection conn;

    // 登录数据库
    if (conn.connecttodb("scott/123", "Simplified Chinese_China.AL32UTF8") != 0) {
        printf("connect database failed.\n%s\n", conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    sqlstatement stmt(&conn);
    stmt.prepare("select memo1 from girls where id=1");

    // 执行 sql 语句对象绑定 clob 字段
    stmt.bindclob();

    // 执行 SQL 语句，一定要判断返回值，0-成功，其它-失败。
    if (stmt.execute() != 0) {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    // 获取一条记录，一定要判断返回值，0-成功，1403-无记录，其它-失败。
    if (stmt.next() != 0) {
        return 0;
    }

    // 把CLOB字段中的内容写入磁盘文件，一定要判断返回值，0-成功，其它-失败。
    if (stmt.lobtofile("/CppIndustrialMeteorologyProject/public/db/oracle/memo_out.txt") != 0) {
        printf("stmt.lobtofile() failed.\n%s\n", stmt.message());
        return -1;
    }

    printf("successfully write clob data from oracle table to disk.\n");

    return 0;
}