#include "_ooci.h"   // 开发框架操作Oracle的头文件。
using namespace idc;

/*
    此程序演示如何处理数字字段的空值
*/

int main(int argc, char* argv[]) {
    connection conn;

    if (conn.connecttodb("scott/123", "Simplified Chinese_China.AL32UTF8") != 0) {
        printf("connect database failed.\n%s\n", conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    // create table tt(c1 number(5),c2 number(5,2));

    sqlstatement stmt(&conn);

    /*
    int c1=0;
    double c2=0;
    stmt.prepare("insert into tt(c1,c2) values(:1,:2)");
    stmt.bindin(1,c1);
    stmt.bindin(2,c2);
    */

    string c1 = "";
    string c2 = "";
    stmt.prepare("insert into tt1(c1,c2) values(:1,:2)");
    stmt.bindin(1, c1, 6);
    stmt.bindin(2, c2, 6);

    printf("sql = %s\n", stmt.sql());

    stmt.execute();

    conn.commit();

    return 0;
}


