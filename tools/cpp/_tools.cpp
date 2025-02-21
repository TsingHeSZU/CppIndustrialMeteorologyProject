#include "_tools.h"


// 实现默认构造函数
ctcols::ctcols() {

}

// 获取指定表的全部字段信息
bool ctcols::allCols(connection& conn, char* tablename) {
    v_all_cols.clear();
    str_all_cols.clear();

    StColumns stcolumns;

    sqlstatement stmt(&conn);
    // 从 USER_TAB_COLUMNS 字典中获取表全部的字段，注意：把结果集转换成小写，数据字典中的表名是大写
    stmt.prepare("\
        select lower(column_name),lower(data_type),data_length from USER_TAB_COLUMNS \
        where table_name=upper(:1) order by column_id");
    stmt.bindin(1, tablename, 30);
    stmt.bindout(1, stcolumns.colname, 30);
    stmt.bindout(2, stcolumns.datatype, 30);
    stmt.bindout(3, stcolumns.collen);

    // 因为查询的是数据字典视图，只有当数据库连接异常时，stmt.execute() 才会失败
    if (stmt.execute() != 0) {
        return false;
    }

    while (true) {
        memset(&stcolumns, 0, sizeof(StColumns));
        if (stmt.next() != 0) {
            break;
        }
        // 列的数据类型，分为 char, date 和 number 三大类
        // 如果业务有需要，可以修改以下代码，增加对更多数据类型的支持

        // 各种字符串类型，rowid 当成字符串处理
        if (strcmp(stcolumns.datatype, "char") == 0) {
            strcpy(stcolumns.datatype, "char");
        }
        if (strcmp(stcolumns.datatype, "nchar") == 0) {
            strcpy(stcolumns.datatype, "char");
        }
        if (strcmp(stcolumns.datatype, "varchar2") == 0) {
            strcpy(stcolumns.datatype, "char");
        }
        if (strcmp(stcolumns.datatype, "nvarchar2") == 0) {
            strcpy(stcolumns.datatype, "char");
        }
        if (strcmp(stcolumns.datatype, "rowid") == 0) {
            strcpy(stcolumns.datatype, "char");
            stcolumns.collen = 18;
        }

        // 日期时间类型，yyyymmddhh24miss
        if (strcmp(stcolumns.datatype, "date") == 0) {
            stcolumns.collen = 14;
        }

        // 数字类型
        if (strcmp(stcolumns.datatype, "number") == 0) {
            strcpy(stcolumns.datatype, "number");
        }
        if (strcmp(stcolumns.datatype, "integer") == 0) {
            strcpy(stcolumns.datatype, "number");
        }
        if (strcmp(stcolumns.datatype, "float") == 0) {
            strcpy(stcolumns.datatype, "number");
        }

        // 如果字段的数据类型不在上面列出来的部分，不做处理
        if ((strcmp(stcolumns.datatype, "char") != 0) &&
            (strcmp(stcolumns.datatype, "number") != 0) &&
            (strcmp(stcolumns.datatype, "date") != 0)) {
            continue;
        }

        // 如果字段类型是 number，把长度设置为 22
        if (strcmp(stcolumns.datatype, "number") == 0) {
            stcolumns.collen = 22;
        }

        str_all_cols = str_all_cols + stcolumns.colname + ",";  // 拼接全部字段字符串
        v_all_cols.push_back(stcolumns);    // 字段信息加入到容器中
    }

    // stmt.rpc()>0 说明从数据字典中查询到了字段信息，删除最后一个多余的半角逗号
    if (stmt.rpc() > 0) {
        deleterchr(str_all_cols, ',');
    }
    return true;
}

// 获取指定表的主键字段信息
bool ctcols::pkCols(connection& conn, char* tablename) {
    v_pk_cols.clear();
    str_pk_cols.clear();

    StColumns stcolumns;
    sqlstatement stmt(&conn);
    // 从 USER_TAB_COLUMNS 字典中获取表全部的字段，注意：把结果集转换成小写，数据字典中的表名是大写
    stmt.prepare("select lower(column_name), position from USER_CONS_COLUMNS\
        where table_name=upper(:1) \
        and constraint_name=(select constraint_name from USER_CONSTRAINTS \
            where table_name=upper(:2) and constraint_type='P' and generated='USER NAME')");
    stmt.bindin(1, tablename, 30);
    stmt.bindin(2, tablename, 30);
    stmt.bindout(1, stcolumns.colname, 30);
    stmt.bindout(2, stcolumns.pkseq);

    // 因为查询的是数据字典视图，只有当数据库连接异常时，stmt.execute() 才会失败
    if (stmt.execute() != 0) {
        return false;
    }

    while (true) {
        memset(&stcolumns, 0, sizeof(StColumns));
        if (stmt.next() != 0) {
            break;
        }

        str_pk_cols = str_pk_cols + stcolumns.colname + ',';    // 拼接主键字符串
        v_pk_cols.push_back(stcolumns);     // 把主键信息放入 v_pk_cols 容器中
    }

    if (stmt.rpc() > 0) {
        deleterchr(str_pk_cols, ',');   // 删除 str_pk_cols 最后一个多余的逗号
    }

    // 更新 v_all_cols 中字段信息结构体的 pkseq 成员，非主键的字段，pkseq 赋值为 0，主键字段，pkseq 赋值为 v_pk_cols 里面结构体变量 pkseq 的值
    for (auto& pk_col : v_pk_cols) {
        for (auto& all_col : v_all_cols) {
            if (strcmp(pk_col.colname, all_col.colname) == 0) {
                all_col.pkseq = pk_col.pkseq;   // 成功修改一个主键的位置后，退出对全部字段信息结构体的遍历
                break;
            }
        }
    }

    return true;
}
