#include "_ooci.h"   // 开发框架操作 Oracle 的头文件
using namespace idc;

void execute_static_sql(connection& conn, sqlstatement& stmt);
void execute_dynamic_sql(connection& conn, sqlstatement& stmt);

int main(int argc, char* argv[]) {
    connection conn;

    if (conn.connecttodb("scott/123", "Simplified Chinese_China.AL32UTF8") != 0) {
        printf("connect database failed.\n%s\n", conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    //sqlstatement stmt;      // 创建执行 sql 语句的对象
    //stmt.connect(&conn);    // 执行 sql 语句的对象指定数据库连接
    sqlstatement stmt(&conn);   // 和上面两行代码等效

    // 执行静态 sql 语句
    execute_static_sql(conn, stmt);

    // 执行动态 sql 语句
    execute_dynamic_sql(conn, stmt);

    // 关闭数据库连接
    conn.disconnect();

    return 0;
}

void execute_static_sql(connection& conn, sqlstatement& stmt) {
    // 准备 sql 语句
    stmt.prepare("\
    insert into girls(id,name,weight,btime,memo) \
        values(0016,'西施', 45.5,to_date('2000-01-01 12:30:35','yyyy-mm-dd hh24:mi:ss'),\
        'static data.')");

    // 执行 SQL 语句，判断返回值，0-成功，其它-失败
    // 失败代码在 stmt.m_cda.rc 中，失败描述在 stmt.m_cda.message 中
    if (stmt.execute() != 0) {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return;
    }

    // stmt.rpc() 是本次 sql 语句执行影响的记录条数
    printf("successfully insert %ld records.\n", stmt.rpc());

    // 创建数据库连接对象 conn 时，默认选择了不自动提交事务，所以执行了修改表记录的 sql 语句后，记得手动提交事务
    conn.commit();
}

void execute_dynamic_sql(connection& conn, sqlstatement& stmt) {
    /*
        关于 oracle 数据库动态 sql 语句执行的一些注意事项
        - 如果字段是字符串类型，绑定的变量可以用 char[], 也可以用 string, 推荐用 char[], 因为 string 在动态扩容的时候，首地址会发生变化
        - 如果字段是字符串类型，bindin() 的第三个参数填字段的长度，太小会有问题，不推荐缺省值
        - 动态 sql 语句的字段也可以填静态的值
        - 绑定的变量，如果很多，一般用结构体来进行封装
    */

    // 准备 sql 语句
    long id;
    char name[31];
    double weight;
    char btime[20];
    char memo[301];
    stmt.prepare("\
        insert into girls(id,name,weight,btime,memo)\
            values(:1,:2,:3,to_date(:4,'yyyy-mm-dd hh24:mi:ss'),:5)");
    stmt.bindin(1, id);
    stmt.bindin(2, name, 30);
    stmt.bindin(3, weight);
    stmt.bindin(4, btime, 19);
    stmt.bindin(5, memo, 300);  // 字符串的长度可以不指定，缺省值是 2000，不建议

    // 对变量赋值，执行 sql 语句
    for (int i = 1;i < 15;++i) {
        id = i;
        memset(name, 0, sizeof(name));
        weight = 45.35 + i;
        memset(btime, 0, sizeof(btime));
        memset(memo, 0, sizeof(memo));

        sprintf(name, "girl%03d", i);
        sprintf(btime, "2025-02-13 15:53:%02d", i);
        sprintf(memo, "dynamic data of sequence %d.", i);

        // 执行 SQL 语句，判断返回值，0-成功，其它-失败
        // 失败代码在 stmt.m_cda.rc 中，失败描述在 stmt.m_cda.message 中
        if (stmt.execute() != 0) {
            printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
            return;
        }

        // stmt.rpc() 是本次 sql 语句执行影响的记录条数
        printf("successfully insert %ld records.\n", stmt.rpc());

    }

    // 创建数据库连接对象 conn 时，默认选择了不自动提交事务，所以执行了修改表记录的 sql 语句后，记得手动提交事务
    conn.commit();
}


