#include "_ooci.h"   // 开发框架操作Oracle的头文件
using namespace idc;

typedef struct GIRLS {
    long id;
    char name[31];
    double weight;
    char btime[20];
    char memo[301];
}GIRLS;

void execute_static_sql(connection& conn, sqlstatement& stmt);
void execute_dynamic_sql(connection& conn, sqlstatement& stmt);

int main(int argc, char* argv[]) {
    // 创建数据库连接对象
    connection conn;

    // 连接数据库
    if (conn.connecttodb("scott/123", "Simplified Chinese_China.AL32UTF8") != 0) {
        printf("connect database failed.\n%s\n", conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    // 创建执行 sql 语句的对象
    sqlstatement stmt(&conn);

    execute_static_sql(conn, stmt);
    execute_dynamic_sql(conn, stmt);

    return 0;
}

void execute_static_sql(connection& conn, sqlstatement& stmt) {
    // 准备 sql 语句
    // 注意，通过编程接口发送的 SQL 命令中不应该包含分号
    stmt.prepare("\
        update girls\
        set name='girl1',weight=45.2,btime=to_date('2008-01-02 12:30:22','yyyy-mm-dd hh24:mi:ss')\
        where id=1"
    );

    // 执行 sql 语句
    if (stmt.execute() != 0) {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return;
    }

    printf("successfully update %ld records.\n", stmt.rpc());

    // 提交事务
    conn.commit();
}

void execute_dynamic_sql(connection& conn, sqlstatement& stmt) {
    // 准备 sql 语句
    GIRLS girls;
    stmt.prepare("\
        update girls\
        set name=:1, weight=:2, btime=to_date(:3,'yyyy-mm-dd hh24:mi:ss')\
        where id=:4"
    );
    stmt.bindin(1, girls.name, 30);
    stmt.bindin(2, girls.weight);
    stmt.bindin(3, girls.btime, 19);
    stmt.bindin(4, girls.id);

    // 对变量赋值，执行 sql 语句
    for (int i = 10;i < 15;++i) {
        memset(&girls, 0, sizeof(girls));
        girls.id = i;
        girls.weight = 35.5 + i;
        sprintf(girls.name, "girl%d", i);
        sprintf(girls.btime, "2025-02-13 15:36:%02d", i);

        // 执行 SQL 语句，判断返回值，0-成功，其它-失败
        // 失败代码在 stmt.m_cda.rc 中，失败描述在 stmt.m_cda.message 中
        if (stmt.execute() != 0) {
            printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
            return;
        }

        // stmt.rpc() 是本次 sql 语句执行影响的记录条数
        printf("successfully update %ld records.\n", stmt.rpc());

    }

    // 创建数据库连接对象 conn 时，默认选择了不自动提交事务，所以执行了修改表记录的 sql 语句后，记得手动提交事务
    conn.commit();
}
