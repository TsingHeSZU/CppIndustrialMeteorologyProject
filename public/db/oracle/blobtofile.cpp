#include "_ooci.h"   // 开发框架操作 Oracle 的头文件
using namespace idc;

int main(int argc, char* argv[]) {
    // 创建数据库连接类对象
    connection conn;

    // 登录数据库：0-成功，其它-失败
    if (conn.connecttodb("scott/123", "Simplified Chinese_China.AL32UTF8") != 0) {
        printf("connect database failed.\n%s\n", conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    // 创建执行 sql 语句的对象
    sqlstatement stmt(&conn);
    stmt.prepare("select pic from girls where id=1");

    // 绑定 blob 字段
    stmt.bindblob();

    // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
    if (stmt.execute() != 0) {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    // 获取一条记录，一定要判断返回值，0-成功，1403-无记录，其它-失败。
    if (stmt.next() != 0) {
        return 0;
    }

    // 把 BLOB 字段中的内容写入磁盘文件，一定要判断返回值，0-成功，其它-失败。
    if (stmt.lobtofile("/CppIndustrialMeteorologyProject/public/db/oracle/pic_out.jpeg") != 0) {
        printf("stmt.lobtofile() failed.\n%s\n", stmt.message());
        return -1;
    }

    printf("successfully write oracle blob to disk binary file.\n");

    return 0;
}