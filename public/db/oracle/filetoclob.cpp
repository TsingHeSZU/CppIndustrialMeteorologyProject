#include "_ooci.h"   // 开发框架操作Oracle的头文件。
using namespace idc;

int main(int argc, char* argv[]) {
    // 创建数据库连接对象
    connection conn;

    // 连接数据库
    if (conn.connecttodb("scott/123", "Simplified Chinese_China.AL32UTF8") != 0) {
        printf("connect database failed.\n%s\n", conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    // 修改 girls 表结构，增加 memo1 字段，用于测试 alter table girls add memo1 clob;
    sqlstatement stmt(&conn);
    stmt.prepare("insert into girls(id,name,memo1) values(1,'girl001',empty_clob())");  // 注意：不可用null代替empty_clob()
    if (stmt.execute() != 0) {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    // 使用游标从 girls 表中提取记录的 memo1 字段
    stmt.prepare("select memo1 from girls where id=1 for update");

    // 执行 sql 语句对象绑定 clob 字段
    stmt.bindclob();

    // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
    if (stmt.execute() != 0) {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    // 获取一条记录，一定要判断返回值，0-成功，1403-无记录，其它-失败。
    if (stmt.next() != 0) {
        return 0;
    }

    // 把磁盘文件 memo_in.txt 的内容写入 CLOB 字段，一定要判断返回值，0-成功，其它-失败。
    if (stmt.filetolob("/CppIndustrialMeteorologyProject/public/db/oracle/memo_in.txt") != 0) {
        // 由于使用了游标，可以修改 select 查询的结果
        printf("stmt.filetolob() failed.\n%s\n", stmt.message()); return -1;
    }

    printf("successfully write txt file to clob.\n");

    // 提交事务
    conn.commit();

    return 0;
}