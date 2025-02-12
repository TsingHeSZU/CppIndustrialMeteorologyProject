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

    // 创建执行 sql 语句的对象
    sqlstatement stmt(&conn);

    stmt.prepare("insert into girls(id,name,pic) values(1,'girl001',empty_blob())");  // 注意：不可用null代替empty_blob()。
    if (stmt.execute() != 0) {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    // 使用游标从 girls 表中提取记录的 pic 字段
    stmt.prepare("select pic from girls where id=1 for update");
    // 绑定 blob 字段
    stmt.bindblob();

    // 执行 SQL 语句，0-成功，其它-失败
    if (stmt.execute() != 0) {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    // 获取一条记录，一定要判断返回值，0-成功，1403-无记录，其它-失败。
    if (stmt.next() != 0) {
        return 0;
    }

    // 把磁盘文件 pic_in.jpeg 的内容写入 BLOB 字段，0-成功，其它-失败。
    if (stmt.filetolob("/CppIndustrialMeteorologyProject/public/db/oracle/pic_in.jpeg") != 0) {
        printf("stmt.filetolob() failed.\n%s\n", stmt.message());
        return -1;
    }

    printf("successfully write binary file to oracle blob.\n");

    conn.commit();

    return 0;
}