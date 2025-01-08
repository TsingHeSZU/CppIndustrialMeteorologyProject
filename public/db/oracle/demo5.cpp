/*
 *  程序名：demo4.cpp，此程序演示执行PL/SQL过程语句。
 *  作者：吴从周。
*/
#include "_ooci.h"   // 开发框架操作Oracle的头文件。
#include <vector>
using namespace idc;

int main(int argc,char *argv[])
{
    connection conn; // 创建数据库连接类的对象。

    if (conn.connecttodb("idc/idcpwd@snorcl11g_128","Simplified Chinese_China.AL32UTF8") != 0)
    {
        printf("connect database failed.\n%s\n",conn.message()); return -1;
    }

    printf("connect database ok.\n");

    vector<string> vcolvalue;
    vcolvalue.resize(8);
    string obtid,cityname,provname,lat,lon,height;
    /*
    OBTID                                     NOT NULL CHAR(5)
 CITYNAME                                  NOT NULL VARCHAR2(30)
 PROVNAME                                  NOT NULL VARCHAR2(30)
 LAT                                       NOT NULL NUMBER(8)
 LON                                       NOT NULL NUMBER(8)
 HEIGHT                                             NUMBER(8)
 UPTTIME                                   NOT NULL DATE
        double weight;     // 超女体重，用double数据类型对应Oracle有小数的number(8,2)。
        char btime[20];     // 报名时间，用char对应Oracle的date，格式：'yyyy-mm-dd hh24:mi:ssi'。
        char memo[301];  // 备注，用char[301]对应Oracle的varchar2(300)。
    } stgirl;
    */

    sqlstatement stmt(&conn);       // 操作SQL语句的对象。
    stmt.prepare("insert into T_ZHOBTCODE1(obtid,cityname,provname,lat,lon,height,upttime,keyid) values(:1,:2,:3,:4,:5,:6,sysdate,seq_zhobtcode1.nextval)");
    stmt.bindin(1,vcolvalue[1],5);
    stmt.bindin(2,vcolvalue[2],30);
    stmt.bindin(3,vcolvalue[3],30);
    stmt.bindin(4,vcolvalue[4],10);
    stmt.bindin(5,vcolvalue[5],10);
    stmt.bindin(6,vcolvalue[6],10);
    //stmt.bindin(1,obtid);
    //stmt.bindin(2,cityname);
    //stmt.bindin(3,provname);
    //stmt.bindin(4,lat);
    //stmt.bindin(5,lon);
    //stmt.bindin(6,height);

    vcolvalue[1]="59287";
    vcolvalue[2]="新宾";
    vcolvalue[3]="辽宁";
    vcolvalue[4]="4144";
    vcolvalue[5]="12503";
    vcolvalue[6]="3284";
    //obtid="59287";
    //cityname="新宾";
    //provname="辽宁";
    //lat="4144";
    //lon="12503";
    //height="3284";

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