/*
 *  程序名：demo3.cpp，此程序演示如何处理数字字段的空值。
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

    // create table tt(c1 number(5),c2 number(5,2));

    sqlstatement stmt(&conn); 

    /*
    int c1=0;
    double c2=0;
    stmt.prepare("insert into tt(c1,c2) values(:1,:2)");
    stmt.bindin(1,c1);
    stmt.bindin(2,c2);
    */

    string c1="";
    string c2="";
    stmt.prepare("insert into tt(c1,c2) values(:1,:2)");
    stmt.bindin(1,c1,6);
    stmt.bindin(2,c2,6);
    
    printf("sql=%s=\n",stmt.sql());

    stmt.execute();

    conn.commit();

    return 0;
}


