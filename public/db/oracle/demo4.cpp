/*
 *  程序名：demo4.cpp，此程序演示执行PL/SQL过程语句。
 *  作者：吴从周。
*/
#include "_ooci.h"   // 开发框架操作Oracle的头文件。
using namespace idc;

int main(int argc,char *argv[])
{
    connection conn; // 创建数据库连接类的对象。

    // 登录数据库，返回值：0-成功，其它-失败。
    // 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
    if (conn.connecttodb("scott/tiger@snorcl11g_128","Simplified Chinese_China.AL32UTF8") != 0)
    {
        printf("connect database failed.\n%s\n",conn.message()); return -1;
    }

    printf("connect database ok.\n");

    struct st_girl
    {
        long id;                 // 超女编号，用long数据类型对应Oracle无小数的number(10)。
        char name[31];     // 超女姓名，用char[31]对应Oracle的varchar2(30)。
        double weight;     // 超女体重，用double数据类型对应Oracle有小数的number(8,2)。
        char btime[20];     // 报名时间，用char对应Oracle的date，格式：'yyyy-mm-dd hh24:mi:ssi'。
        char memo[301];  // 备注，用char[301]对应Oracle的varchar2(300)。
    } stgirl;

    // 准备PL/SQL语句，如果SQL语句有错误，prepare()不会返回失败，所以，prepare()不需要判断返回值。
    sqlstatement stmt(&conn);       // 操作SQL语句的对象。
    // PL/SQL语句的优点：减少了客户端与数据库的通讯次数，提高了效率。
    stmt.prepare("\
        begin\
            delete from girls where id=:1;\
            insert into girls(id,name) values(:2,:3);\
            update girls set weight=:4 where id=:5;\
        end;");              
    stmt.bindin(1,stgirl.id);
    stmt.bindin(2,stgirl.id);
    stmt.bindin(3,stgirl.name,30);
    stmt.bindin(4,stgirl.weight);
    stmt.bindin(5,stgirl.id);

    memset(&stgirl,0,sizeof(struct st_girl));
    stgirl.id=1;
    strcpy(stgirl.name,"冰冰");
    stgirl.weight=49.5;

    // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
    // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n",stmt.sql(),stmt.message()); return -1;
    }

    printf("exec pl/sql ok.\n");

    printf("影响了%ld条记录。\n",stmt.rpc());    // pl/sql中最后一条SQL语句记录影响记录的行数。

    conn.commit();

    return 0;
}