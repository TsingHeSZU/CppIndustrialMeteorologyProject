#include "_ooci.h"   // 开发框架操作Oracle的头文件
using namespace idc;

/*
    此程序演示 char 和 varchar2 的问题
*/

int main(int argc, char* argv[]) {
    // 创建数据库连接类的对象
    connection conn;

    if (conn.connecttodb("scott/123", "Simplified Chinese_China.AL32UTF8") != 0) {
        printf("connect database failed.\n%s\n", conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    // create table tt(c1 char(5),c2 varchar2(5));
    // insert into tt values('abc','abc');

    sqlstatement stmt(&conn);
    string str = "abc  ";

    int ccount = 0;
    stmt.prepare("select count(*) from tt where c1=:1");
    stmt.bindin(1, str, 5);
    stmt.bindout(1, ccount);

    printf("sql = %s\n", stmt.sql());

    if (stmt.execute() != 0) {
        printf("stmt.execute() failed.\n%s\n", stmt.message());
    }

    stmt.next();

    printf("ccount = %d\n", ccount);

    return 0;
}