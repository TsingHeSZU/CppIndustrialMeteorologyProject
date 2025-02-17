#include "idcapp.h"

CZHOBTMIND::CZHOBTMIND(connection& conn, clogfile& logfile) :m_conn(conn), m_logfile(logfile) {

}

CZHOBTMIND::~CZHOBTMIND() {

}

// 把从文件中读到的一行数据拆分到 m_zhobtmind 结构体中
bool CZHOBTMIND::splitBuffer(const string& str_buffer, const bool is_xml) {
    // 解析行的内容（*.xml 和 *.csv 的方法不同），把数据存放到结构体中
    memset(&this->m_zhobtmind, '\0', sizeof(StZHOBTMIND));
    if (is_xml == true) {
        getxmlbuffer(str_buffer, "obtid", this->m_zhobtmind.obtid, 5);
        getxmlbuffer(str_buffer, "ddatetime", this->m_zhobtmind.ddatetime, 14);
        char tmp[11];
        getxmlbuffer(str_buffer, "t", tmp, 10);
        if (strlen(tmp) > 0) {
            snprintf(this->m_zhobtmind.t, 10, "%d", (int)(atof(tmp) * 10));
        }

        getxmlbuffer(str_buffer, "p", tmp, 10);
        if (strlen(tmp) > 0) {
            snprintf(this->m_zhobtmind.p, 10, "%d", (int)(atof(tmp) * 10));
        }

        getxmlbuffer(str_buffer, "rh", this->m_zhobtmind.rh, 10);
        getxmlbuffer(str_buffer, "wd", this->m_zhobtmind.wd, 10);
        getxmlbuffer(str_buffer, "ws", tmp, 10);
        if (strlen(tmp) > 0) {
            snprintf(this->m_zhobtmind.ws, 10, "%d", (int)(atof(tmp) * 10));
        }
        getxmlbuffer(str_buffer, "r", tmp, 10);
        if (strlen(tmp) > 0) {
            snprintf(this->m_zhobtmind.r, 10, "%d", (int)(atof(tmp) * 10));
        }
        getxmlbuffer(str_buffer, "vis", tmp, 10);  if (strlen(tmp) > 0) {
            snprintf(this->m_zhobtmind.vis, 10, "%d", (int)(atof(tmp) * 10));
        }
    }
    else {
        ccmdstr cmdstr;
        cmdstr.splittocmd(str_buffer, ",");
        cmdstr.getvalue(0, this->m_zhobtmind.obtid, 5);

        cmdstr.getvalue(1, this->m_zhobtmind.ddatetime, 14);

        char tmp[11];
        cmdstr.getvalue(2, tmp, 10);
        if (strlen(tmp) > 0) {
            snprintf(this->m_zhobtmind.t, 10, "%d", (int)(atof(tmp) * 10));
        }

        cmdstr.getvalue(3, tmp, 10);
        if (strlen(tmp) > 0) {
            snprintf(this->m_zhobtmind.p, 10, "%d", (int)(atof(tmp) * 10));
        }

        cmdstr.getvalue(4, this->m_zhobtmind.rh, 10);

        cmdstr.getvalue(5, this->m_zhobtmind.wd, 10);


        cmdstr.getvalue(6, tmp, 10);
        if (strlen(tmp) > 0) {
            snprintf(m_zhobtmind.ws, 10, "%d", (int)(atof(tmp) * 10));
        }

        cmdstr.getvalue(7, tmp, 10);
        if (strlen(tmp) > 0) {
            snprintf(m_zhobtmind.r, 10, "%d", (int)(atof(tmp) * 10));
        }

        cmdstr.getvalue(8, tmp, 10);
        if (strlen(tmp) > 0) {
            snprintf(m_zhobtmind.vis, 10, "%d", (int)(atof(tmp) * 10));
        }
    }

    this->m_buffer = str_buffer;
    return true;
}

// 把 m_zhobtmind 结构体中的数据插入到 T_ZHOBTMIND 表中
bool CZHOBTMIND::insertTable() {
    if (this->m_stmt.isopen() == false) {

        this->m_stmt.connect(&this->m_conn);

        // 准备操作表的 sql 语句，绑定输入参数
        this->m_stmt.prepare("insert into T_ZHOBTMIND(OBTID,DDATETIME,T,P,RH,WD,WS,R,VIS,KEYID)"\
            "values(:1,TO_DATE(:2,'yyyymmddhh24miss'),:3,:4,:5,:6,:7,:8,:9,SEQ_ZHOBTMIND.nextval)");
        this->m_stmt.bindin(1, this->m_zhobtmind.obtid, 5);
        this->m_stmt.bindin(2, this->m_zhobtmind.ddatetime, 14);
        this->m_stmt.bindin(3, this->m_zhobtmind.t, 10);
        this->m_stmt.bindin(4, this->m_zhobtmind.p, 10);
        this->m_stmt.bindin(5, this->m_zhobtmind.rh, 10);
        this->m_stmt.bindin(6, this->m_zhobtmind.wd, 10);
        this->m_stmt.bindin(7, this->m_zhobtmind.ws, 10);
        this->m_stmt.bindin(8, this->m_zhobtmind.r, 10);
        this->m_stmt.bindin(9, this->m_zhobtmind.vis, 10);
    }

    // 把解析后的数据插入到数据库表中
    if (this->m_stmt.execute() != 0) {
        // sql 语句执行失败的原因有两个，一是记录重复，二是数据内容非法
        // 如果失败的原因是数据内容非法，记录日之后继续，如果数据重复，不必记录日志，且继续
        if (this->m_stmt.rc() != 1) {
            this->m_logfile.write("str_buffer = %s\n", this->m_buffer.c_str());
            this->m_logfile.write("this->m_stmt.execute() failed.\n%s\n%s\n", this->m_stmt.sql(), this->m_stmt.message());
        }

        return false;
    }
    return true;
}
