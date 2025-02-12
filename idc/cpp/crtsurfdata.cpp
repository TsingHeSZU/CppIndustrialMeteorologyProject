#include"_public.h"
using namespace idc;

/*
    站点参数结构体：
    省、站号、站名、维度、经度、海拔高度
    string 操作方便，自动扩容，但是操作数据库时，没有优势
*/
typedef struct StationCode {
    char prov_name[31];     // 省
    char station_id[11];    // 站点编号
    char station_name[31];  // 站名
    double latitude;        // 维度
    double longitude;       // 经度
    double height;          // 海拔
}StationCode;

/*
    气象站观测数据结构体
*/
typedef struct StationSurfData {
    char station_id[11];    // 站点编号
    char d_datatime[15];    // 数据时间：格式yyyymmddhh24miss，精确到分钟，秒固定填00
    int t;      // 气温，单位，0.1 摄氏度，写入文件保存时，需要除10
    int p;      // 气压，0.1 百帕
    int u;      // 相对湿度，0 - 100 之间的值
    int wd;     // 风向，角度，0 - 360 之间的值
    int wf;     // 风速，单位 0.1 m/s
    int r;      // 降雨量，0.1 mm
    int vis;    // 能见度，0.1 米
}StationSurfData;

list<StationCode> stcode_list;  // 存放站点参数
list<StationSurfData> data_list;    // 存放观测数据
Cpactive pactive;       // 进程心跳信息

clogfile logfile;       // 日志文件
char str_ddatatime[15];     // 存储当前系统时间

/*
    把站点参数文件加载到 stcode_list 容器中
    - inifile: 参数文件路径
*/
bool loadStationCode(const string& inifile);

/*
    根据 stcode_list 容器中的站点参数，生成站点观测数据，存放在 data_list 中
*/
void crtSurfData();

/*
    把容器 data_list 中的气象观测数据写入文件
    - outpath: 观测数据写入文件的路径（不包括文件名）
    - datafmt: 写入文件的格式
*/
bool writeCrtSurfData(const string& outpath, const string& datafmt);

void EXIT(int sig);     // 程序退出和信号 2 、15 的处理函数

int main(int argc, char* argv[]) {
    // 站点参数文件  生成的测试数据存放的目录 本程序运行的日志
    if (argc != 5)
    {
        // 如果参数非法，给出帮助文档
        cout << "Using format: ./crtsurfdata inifile outpath logfile datafmt\n";
        cout << "Examples: /CppIndustrialMeteorologyProject/tools/bin/procctl 60 /CppIndustrialMeteorologyProject/idc/bin/crtsurfdata /CppIndustrialMeteorologyProject/idc/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log csv,xml,json\n";

        cout << "本程序用于生成气象站点观测的分钟数据, 程序每分钟运行一次, 由调度模块启动。\n";
        cout << "inifile: 气象站点参数文件名。\n";
        cout << "outpath: 气象站点观测数据文件存放的目录。\n";
        cout << "logfile: 本程序运行的日志文件名。\n";
        cout << "datafmt: 输出数据文件的格式, 支持 csv, xml和 json, 中间用逗号分隔。\n";
        return -1;
    }

    // 设置信号，在 shell 状态下可用 "kill + 进程号" 正常终止进程
    // 不要用 "kill -9 + 进程号" 强行终止 
    closeioandsignal(true);     // 关闭 0、1、2 I/O 和忽略全部的信号
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    pactive.addProcInfo(10, "crtsurfdata");     // 把当前进程的心跳加入共享内存

    if (!logfile.open(argv[3])) {
        cout << "logfile.open(" << argv[3] << ") failed.";
        return -1;
    }

    logfile.write("crtsurfdata 开始运行。\n");

    // 业务逻辑

    // 1. 从站点参数文件中加载站点参数，保存到 stcode_list 容器中
    if (loadStationCode(argv[1]) == false) {
        logfile.write("load station code file %s failed.\n", argv[1]);
        EXIT(-1);
    }

    memset(str_ddatatime, 0, sizeof(str_ddatatime));
    ltime(str_ddatatime, "yyyymmddhh24miss");       // 获取系统当前时间
    strncpy(str_ddatatime + 12, "00", 2);        // 数据时间中的秒固定填 00

    // 2. 根据站点参数，生成站点观测数据（随机数）
    crtSurfData();

    // 3. 把站点观测数据 data_list 保存到文件中
    if (strstr(argv[4], "csv")) {
        writeCrtSurfData(argv[2], "csv");
    }
    if (strstr(argv[4], "xml")) {
        writeCrtSurfData(argv[2], "xml");
    }
    if (strstr(argv[4], "json")) {
        writeCrtSurfData(argv[2], "json");
    }

    logfile.write("crtsurfdata 运行结束。\n\n");
    return 0;
}

/*
    把站点参数文件加载到 stcode_list 容器中
    - inifile: 参数文件路径
*/
bool loadStationCode(const string& inifile) {
    cifile ifile;   // 读取文件对象
    if (ifile.open(inifile) == false) {
        // 写入文件之前，cpp 字符串风格转换为 c 字符串风格，可以防止乱码
        logfile.write("ifile.open( %s ) failed.\n", inifile.c_str());
        return false;
    }

    string str_buffer;      // 存放从文件中读取的每一行
    ifile.readline(str_buffer);     // 站点参数文件第一行是标题，无效

    ccmdstr cmdstr;     // 用于拆分从参数文件中读取的行
    StationCode st_code;    // 站点参数结构体

    while (ifile.readline(str_buffer)) {
        // logfile.write("str_buffer = %s\n", str_buffer.c_str());  // 测试代码

        memset(&st_code, 0, sizeof(st_code));

        cmdstr.splittocmd(str_buffer, ",");
        cmdstr.getvalue(0, st_code.prov_name, 30);  // 省
        cmdstr.getvalue(1, st_code.station_id, 10);   // 站点编号
        cmdstr.getvalue(2, st_code.station_name, 30);   // 站点名称
        cmdstr.getvalue(3, st_code.latitude);     // 维度
        cmdstr.getvalue(4, st_code.longitude);      // 经度
        cmdstr.getvalue(5, st_code.height);     // 海拔高度

        stcode_list.push_back(st_code);     // 把站点参数存入到 list 容器中
    }

    // 这里不需要手动关闭打开的 inifile，cifile 类的析构函数会关闭文件

    // 把容器中全部的数据写入日志，测试代码
    // for (auto& st : stcode_list) {
    //     logfile.write("prov_name = %s, station_id = %s, station_name = %s, latitude = %.2f, longitude = %.2f, height = %.2f\n",
    //         st.prov_name, st.station_id, st.station_name, st.latitude, st.longitude, st.height);
    // }

    return true;
}

/*
    根据 stcode_list 容器中的站点参数，生成站点观测数据，存放在 data_list 中
*/
void crtSurfData() {
    srand(time(0));     // 随机数种子
    StationSurfData st_surfdata;    // 观测数据结构体

    for (auto& st_code : stcode_list) {
        memset(&st_surfdata, 0, sizeof(st_surfdata));

        // 填充观测数据的结构体成员
        strcpy(st_surfdata.station_id, st_code.station_id);     // 站点编号
        strcpy(st_surfdata.d_datatime, str_ddatatime);       // 数据时间
        st_surfdata.t = rand() % 350;       // 气温
        st_surfdata.p = rand() % 265 + 10000;   // 气压： 0.1 百帕
        st_surfdata.u = rand() % 101;       // 相对湿度，0 - 100 之间的值
        st_surfdata.wd = rand() % 360;      // 风向：0 - 360 之间的值
        st_surfdata.wf = rand() % 150;      // 风速：单位 0.1 m/s
        st_surfdata.r = rand() % 16;        // 降雨量：0.1 mm
        st_surfdata.vis = rand() % 5001 + 100000;   // 能见度：0.1 m
        data_list.push_back(st_surfdata);
    }

    // 生成的观测数据写入日志中，测试代码
    // for (auto& st_data : data_list) {
    //     logfile.write("%s, %s, %.1f, %.1f, %d, %d, %.1f, %.1f, %.1f\n",
    //         st_data.station_id, st_data.d_datatime, st_data.t / 10.0, st_data.p / 10.0, st_data.u,
    //         st_data.wd, st_data.wf / 10.0, st_data.r / 10.0, st_data.vis / 10.0);
    // }
}

/*
    把容器 data_list 中的气象观测数据写入文件
    - outpath: 观测数据写入文件的路径（不包括文件名）
    - datafmt: 写入文件的格式
*/
bool writeCrtSurfData(const string& outpath, const string& datafmt) {
    // 拼接生成数据的文件名，例如： /tmp/idc/surfdata/SURF_ZH_20210629092200_2254.CSV
    string str_file_name = outpath + "/" + "SURF_ZH_" + str_ddatatime + "_" + to_string(getpid()) + "." + datafmt;

    cofile ofile;   // 写入数据文件对象
    if (!ofile.open(str_file_name)) {
        logfile.write("ofile.open( %s ) failed.\n", str_file_name.c_str());
        return false;
    }

    // 把 data_list 容器中的观测数据写入文件，支持 csv, xml 和 json 三种格式
    if (datafmt == "csv") {
        ofile.writeline("站点编号, 数据时间, 气温, 气压, 相对湿度, 风向, 风速, 降雨量, 能见度\n");
    }
    if (datafmt == "xml") {
        ofile.writeline("<data>\n");
    }
    if (datafmt == "json") {
        ofile.writeline("{\"data\":[\n");
    }

    // 遍历存放观测数据的 data_list 容器
    for (auto& st_data : data_list) {
        if (datafmt == "csv") {
            ofile.writeline("%s,%s,%.1f,%.1f,%d,%d,%.1f,%.1f,%.1f\n",
                st_data.station_id, st_data.d_datatime, st_data.t / 10.0, st_data.p / 10.0,
                st_data.u, st_data.wd, st_data.wf / 10.0, st_data.r / 10.0, st_data.vis / 10.0);
        }
        if (datafmt == "xml") {
            ofile.writeline("<station_id>%s</station_id><d_datatime>%s</d_datatime><t>%.1f</t><p>%.1f</p><u>%d</u><wd>%d</wd><wf>%.1f</wf><r>%.1f</r><vis>%.1f</vis><endl/>\n",
                st_data.station_id, st_data.d_datatime, st_data.t / 10.0, st_data.p / 10.0,
                st_data.u, st_data.wd, st_data.wf / 10.0, st_data.r / 10.0, st_data.vis / 10.0);
        }
        if (datafmt == "json") {
            ofile.writeline("{\"station_id\":\"%s\",\"d_datatime\":\"%s\",\"t\":\"%.1f\",\"p\":\"%.1f\",\"u\":\"%d\",\"wd\":\"%d\",\"wf\":\"%.1f\",\"r\":\"%.1f\",\"vis\":\"%.1f\"}",
                st_data.station_id, st_data.d_datatime, st_data.t / 10.0, st_data.p / 10.0,
                st_data.u, st_data.wd, st_data.wf / 10.0, st_data.r / 10.0, st_data.vis / 10.0);

            // 注意，json 文件的最后一条记录不需要逗号，用以下代码特殊处理
            static int ii = 0;      // 已写入数据行数的计数器
            if (ii < data_list.size() - 1) {
                // 如果不是最后一行
                ofile.writeline(",\n");
                ++ii;
            }
            else {
                ofile.writeline("\n");
            }
        }

    }

    if (datafmt == "xml") {
        ofile.writeline("</data>\n");
    }
    if (datafmt == "json") {
        ofile.writeline("]}\n");
    }

    // 关闭临时文件，并改名为正式的文件
    ofile.closeandrename();

    logfile.write("生成观测数据文件 %s 成功, 数据时间 %s, 记录数 %d。\n", str_file_name.c_str(), str_ddatatime, data_list.size());

    return true;
}

// 程序退出和信号 2、15 的处理函数
void EXIT(int sig) {
    logfile.write("程序退出, sig = %d\n\n", sig);
    exit(0);
}