#include "_ooci.h"   // 开发框架操作Oracle的头文件
using namespace idc;

/*
    此程序演示执行PL/SQL过程语句
*/

int main(int argc, char* argv[]) {
    connection conn;

    if (conn.connecttodb("scott/123", "Simplified Chinese_China.AL32UTF8") != 0) {
        printf("connect database failed.\n%s\n", conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    struct st_girl
    {
        long id;
        char name[31];
        double weight;
        char btime[20];
        char memo[301];
    } stgirl;

    sqlstatement stmt(&conn);
    // PL/SQL语句的优点：减少了客户端与数据库的通讯次数，提高了效率
    // 准备PL/SQL语句，如果SQL语句有错误，prepare()不会返回失败，所以，prepare()不需要判断返回值
    stmt.prepare("\
        begin\
            delete from girls where id=:1;\
            insert into girls(id,name) values(:2,:3);\
            update girls set weight=:4 where id=:5;\
        end;");
    stmt.bindin(1, stgirl.id);
    stmt.bindin(2, stgirl.id);
    stmt.bindin(3, stgirl.name, 30);
    stmt.bindin(4, stgirl.weight);
    stmt.bindin(5, stgirl.id);

    memset(&stgirl, 0, sizeof(struct st_girl));
    stgirl.id = 1;
    strcpy(stgirl.name, "girl001");
    stgirl.weight = 49.5;

    if (stmt.execute() != 0) {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    printf("exec pl/sql ok.\n");

    // pl/sql中最后一条SQL语句记录影响记录的行数
    printf("pl/sql last sql influences %ld records.\n", stmt.rpc());

    conn.commit();
    return 0;
}