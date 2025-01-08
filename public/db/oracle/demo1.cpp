/*
 *  程序名：demo1.cpp，此程序演示静态SQL语句的安全性问题（SQL注入）。
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

    // create table operator(username varchar2(30),passwd varchar2(30));
    // insert into operator values('wucz','oracle');

    string username,passwd;
    int ccount=0;

    username="wucz";   // 假设操作员输入了用户名。
    //username="' or 1=1 or ''='";
    passwd="oracle";     // 假设操作员输入了密码。

    sqlstatement stmt(&conn); 
    //stmt.prepare("select count(*) from operator where username='%s' and passwd='%s'",username.c_str(),passwd.c_str());
    stmt.prepare("select count(*) from operator where username=:1 and passwd=:2");
    stmt.bindin(1,username,30);
    stmt.bindin(2,passwd,30);
    stmt.bindout(1,ccount);

    printf("sql=%s=\n",stmt.sql());
    stmt.execute();
    stmt.next();
    if (ccount == 0) 
       printf("登录失败。\n");
    else
       printf("登录成功。\n");

    return 0;
}