#ifndef IDCAPP_H
#define IDCAPP_H

#include "_public.h"
#include "_ooci.h"
using namespace idc;

// 气象站点观测数据结构体
typedef struct StZHOBTMIND {
    char obtid[11];         // 站点代码
    char ddatetime[21];     // 数据时间，精确到分钟
    char t[11];             // 温度，单位：0.1 摄氏度
    char p[11];             // 气压，单位：0.1 百帕
    char rh[11];            // 相对湿度，0-100 之间
    char wd[11];            // 风向，0-360 之间
    char ws[11];            // 风速，单位 0.1 m/s
    char r[11];             // 降雨量，0.1 mm
    char vis[11];           // 能见度，0.1 m
}StZHOBTMIND;

// 全国气象观测数据操作类
class CZHOBTMIND {
private:

    connection& m_conn;         // 数据库连接
    clogfile& m_logfile;        // 日志文件
    sqlstatement m_stmt;        // 插入表的 sql
    string m_buffer;            // 从文件中读到的一行
    StZHOBTMIND m_zhobtmind;    // 全国气象观测数据结构体变量

public:
    CZHOBTMIND(connection& conn, clogfile& logfile);
    ~CZHOBTMIND();

    // 把从文件中读到的一行数据拆分到 m_zhobtmind 结构体中
    bool splitBuffer(const string& str_buffer, const bool is_xml);

    // 把 m_zhobtmind 结构体中的数据插入到 T_ZHOBTMIND 表中
    bool insertTable();

};

#endif