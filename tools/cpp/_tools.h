#ifndef _TOOLS_H
#define _TOOLS_H
#include "_public.h"
#include "_ooci.h"
using namespace idc;

// 表的列（字段）信息的结构体
typedef struct StColumns {
    char colname[31];       // 列名
    char datatype[31];      // 列的数据类型，分为 number, date 和 char 三大类
    int collen;             // 列的长度，number 固定 22，date 固定 19，char 的长度由表结构决定
    int pkseq;              // 如果列是主键的字段，存放主键字段的顺序，从 1 开始，不是主键取值为 0
}StColumns;

// 获取表全部列和主键列信息的类
class ctcols {
private:

public:
    vector<StColumns> v_all_cols;   // 存放全部字段信息的容器
    vector<StColumns> v_pk_cols;    // 存放主键字段信息的容器

    string str_all_cols;    // 全部字段名列表，以字符串存放，中间用半角的逗号分隔
    string str_pk_cols;     // 主键字段名列表，以字符串存放，中间用半角的逗号分隔

    ctcols();
    // 获取指定表的全部字段信息
    bool allCols(connection& conn, char* tablename);

    // 获取指定表的主键字段信息
    bool pkCols(connection& conn, char* tablename);
};

#endif