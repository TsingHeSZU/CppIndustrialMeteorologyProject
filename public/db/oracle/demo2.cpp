/*
 *  程序名：demo2.cpp，此程序演示char和varchar2的问题。
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

    // create table tt(c1 char(5),c2 varchar2(5));
    // insert into tt values('abc','abc');

    sqlstatement stmt(&conn); 
    string str="abc";

    int ccount=0;
    stmt.prepare("select count(*) from tt where c1='%s'",str.c_str());
    //stmt.prepare("select count(*) from tt where c1=:1");
    //stmt.bindin(1,str,5);
    stmt.bindout(1,ccount);

    printf("sql=%s=\n",stmt.sql());
    stmt.execute();
    stmt.next();
    printf("ccount=%d\n",ccount);

    return 0;
}