/****************************************************************************************/
/*   程序名：_public.h，此程序是公共函数和类的声明文件。                                            */
/*   作者：吴从周                                                                                                             */
/****************************************************************************************/
#ifndef __PUBLIC_HH
#define __PUBLIC_HH 1

#include "_cmpublic.h"    // C/C++、Linux编程常用的头文件，新的头文件请在_cmpublic.h中添加。

using namespace std;

namespace idc
{

///////////////////////////////////// /////////////////////////////////////
// C++风格字符串操作的若干函数。
// 删除字符串左边指定的字符。
// str：待处理的字符串。
// cc：需要删除的字符，缺省删除空格。
char*     deletelchr(char* str, const int cc=' ');
string& deletelchr(string &str, const int cc=' ');

// 删除字符串右边指定的字符。
// str：待处理的字符串。
// cc：需要删除的字符，缺省删除空格。
char*     deleterchr(char *str,const int cc=' ');
string& deleterchr(string &str,const int cc=' ');

// 删除字符串左右两边指定的字符。
// str：待处理的字符串。
// chr：需要删除的字符，缺省删除空格。
char*     deletelrchr(char *str,const int cc=' ');
string& deletelrchr(string &str,const int cc=' ');

// 把字符串中的小写字母转换成大写，忽略不是字母的字符。
// str：待转换的字符串。
char*     toupper(char *str);
string& toupper(string &str);

// 把字符串中的大写字母转换成小写，忽略不是字母的字符。
// str：待转换的字符串。
char*     tolower(char *str);
string& tolower(string &str);

// 字符串替换函数。
// 在字符串str中，如果存在字符串str1，就替换为字符串str2。
// str：待处理的字符串。
// str1：旧的内容。
// str2：新的内容。
// bloop：是否循环执行替换。
// 注意：
// 1、如果str2比str1要长，替换后str会变长，所以必须保证str有足够的空间，否则内存会溢出（C++风格字符串不存在这个问题）。
// 2、如果str2中包含了str1的内容，且bloop为true，这种做法存在逻辑错误，replacestr将什么也不做。
// 3、如果str2为空，表示删除str中str1的内容。
bool replacestr(char *str   ,const string &str1,const string &str2,const bool bloop=false);
bool replacestr(string &str,const string &str1,const string &str2,const bool bloop=false);

// 从一个字符串中提取出数字、符号和小数点，存放到另一个字符串中。
// src：原字符串。
// dest：目标字符串。
// bsigned：是否提取符号（+和-），true-包括；false-不包括。
// bdot：是否提取小数点（.），true-包括；false-不包括。
// 注意：src和dest可以是同一个变量。
char*     picknumber(const string &src,char *dest,const bool bsigned=false,const bool bdot=false);
string& picknumber(const string &src,string &dest,const bool bsigned=false,const bool bdot=false);
string    picknumber(const string &src,const bool bsigned=false,const bool bdot=false);

// 正则表达式，判断一个字符串是否匹配另一个字符串。
// str：需要判断的字符串，是精确表示的，如文件名"_public.cpp"。
// rules：匹配规则的表达式，用星号"*"代表任意字符，多个表达式之间用半角的逗号分隔，如"*.h,*.cpp"。
// 注意：1）str参数不需要支持"*"，rules参数支持"*"；2）函数在判断str是否匹配rules的时候，会忽略字母的大小写。
bool matchstr(const string &str,const string &rules);
///////////////////////////////////// /////////////////////////////////////

///////////////////////////////////// /////////////////////////////////////
// ccmdstr类用于拆分有分隔符的字符串。
// 字符串的格式为：字段内容1+分隔符+字段内容2+分隔符+字段内容3+分隔符+...+字段内容n。
// 例如："messi,10,striker,30,1.72,68.5,Barcelona"，这是足球运动员梅西的资料。
// 包括：姓名、球衣号码、场上位置、年龄、身高、体重和效力的俱乐部，字段之间用半角的逗号分隔。
class ccmdstr
{
private:
    vector<string> m_cmdstr;  // 存放拆分后的字段内容。

    ccmdstr(const ccmdstr &) = delete;                      // 禁用拷贝构造函数。
    ccmdstr &operator=(const ccmdstr &) = delete;  // 禁用赋值函数。
public:
    ccmdstr()  { } // 构造函数。
    ccmdstr(const string &buffer,const string &sepstr,const bool bdelspace=false);

    const string& operator[](int ii) const     // 重载[]运算符，可以像访问数组一样访问m_cmdstr成员。
    {
        return m_cmdstr[ii];
    }

    // 把字符串拆分到m_cmdstr容器中。
    // buffer：待拆分的字符串。
    // sepstr：buffer中采用的分隔符，注意，sepstr参数的数据类型不是字符，是字符串，如","、" "、"|"、"~!~"。
    // bdelspace：拆分后是否删除字段内容前后的空格，true-删除；false-不删除，缺省不删除。
    void splittocmd(const string &buffer,const string &sepstr,const bool bdelspace=false);

    // 获取拆分后字段的个数，即m_cmdstr容器的大小。
    int size() const { return m_cmdstr.size(); }
    int cmdcount() const { return m_cmdstr.size(); }      // 兼容以前的项目。

    // 从m_cmdstr容器获取字段内容。
    // ii：字段的顺序号，类似数组的下标，从0开始。
    // value：传入变量的地址，用于存放字段内容。
    // 返回值：true-成功；如果ii的取值超出了m_cmdstr容器的大小，返回失败。
    bool getvalue(const int ii,string &value,const int ilen=0) const;      // C++风格字符串。视频中没有第三个参数，加上第三个参数更好。
    bool getvalue(const int ii,char *value,const int ilen=0) const;          // C风格字符串，ilen缺省值为0-全部长度。 
    bool getvalue(const int ii,int  &value) const;                                    // int整数。
    bool getvalue(const int ii,unsigned int &value) const;                     // unsigned int整数。
    bool getvalue(const int ii,long &value) const;                                  // long整数。
    bool getvalue(const int ii,unsigned long &value) const;                  // unsigned long整数。
    bool getvalue(const int ii,double &value) const;                              // 双精度double。
    bool getvalue(const int ii,float &value) const;                                  // 单精度float。
    bool getvalue(const int ii,bool &value) const;                                  // bool型。

    ~ccmdstr(); // 析构函数。
};

// 重载<<运算符，输出ccmdstr::m_cmdstr中的内容，方便调试。
ostream& operator<<(ostream& out, const ccmdstr& cc);
///////////////////////////////////// /////////////////////////////////////

///////////////////////////////////// /////////////////////////////////////
// 解析xml格式字符串的函数族。
// xml格式的字符串的内容如下：
// <filename>/tmp/_public.h</filename><mtime>2020-01-01 12:20:35</mtime><size>18348</size>
// <filename>/tmp/_public.cpp</filename><mtime>2020-01-01 10:10:15</mtime><size>50945</size>
// xmlbuffer：待解析的xml格式字符串。
// fieldname：字段的标签名。
// value：传入变量的地址，用于存放字段内容，支持bool、int、insigned int、long、
//       unsigned long、double和char[]。
// 注意：当value参数的数据类型为char []时，必须保证value数组的内存足够，否则可能发生内存溢出的问题，
//           也可以用ilen参数限定获取字段内容的长度，ilen的缺省值为0，表示不限长度。
// 返回值：true-成功；如果fieldname参数指定的标签名不存在，返回失败。
bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,string &value,const int ilen=0);    // 视频中没有第三个参数，加上第三个参数更好。
bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,char *value,const int ilen=0);
bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,bool &value);
bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,int  &value);
bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,unsigned int &value);
bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,long &value);
bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,unsigned long &value);
bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,double &value);
bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,float &value);
///////////////////////////////////// /////////////////////////////////////

// C++格式化输出函数模板。
template< typename... Args >
bool sformat(string &str,const char* fmt, Args... args ) 
{
    int len = snprintf( nullptr, 0, fmt, args... );      // 得到格式化输出后字符串的总长度。
    if (len < 0) return false;                                  // 如果调用snprintf失败，返回-1。
    if (len == 0) { str.clear(); return true; }            // 如果调用snprintf返回0，表示格式化输出的内容为空。

    str.resize(len);                                                 // 为string分配内存。
    snprintf(&str[0], len + 1, fmt, args... );           // linux平台第二个参数是len+1，windows平台是len。
    return true;
}
template< typename... Args >
string sformat(const char* fmt, Args... args ) 
{
    string str;

    int len = snprintf( nullptr, 0, fmt, args... );      // 得到格式化后字符串的长度。
    if (len < 0) return str;              // 如果调用snprintf失败，返回-1。
    if (len == 0) return str;           // 如果调用snprintf返回0，表示格式化输出的内容为空。;

    str.resize(len);                                                // 为string分配内存。
    snprintf(&str[0], len + 1, fmt, args... );          // linux平台第二个参数是len+1，windows平台是len。
    return str;
}

///////////////////////////////////// /////////////////////////////////////
// 时间操作的若干函数。
/*
  取操作系统的时间（用字符串表示）。
  strtime：用于存放获取到的时间。
  timetvl：时间的偏移量，单位：秒，0是缺省值，表示当前时间，30表示当前时间30秒之后的时间点，-30表示当前时间30秒之前的时间点。
  fmt：输出时间的格式，fmt每部分的含义：yyyy-年份；mm-月份；dd-日期；hh24-小时；mi-分钟；ss-秒，
  缺省是"yyyy-mm-dd hh24:mi:ss"，目前支持以下格式：
  "yyyy-mm-dd hh24:mi:ss"
  "yyyymmddhh24miss"
  "yyyy-mm-dd"
  "yyyymmdd"
  "hh24:mi:ss"
  "hh24miss"
  "hh24:mi"
  "hh24mi"
  "hh24"
  "mi"
  注意：
    1）小时的表示方法是hh24，不是hh，这么做的目的是为了保持与数据库的时间表示方法一致；
    2）以上列出了常用的时间格式，如果不能满足你应用开发的需求，请修改源代码timetostr()函数增加更多的格式支持；
    3）调用函数的时候，如果fmt与上述格式都匹配，strtime的内容将为空。
    4）时间的年份是四位，其它的可能是一位和两位，如果不足两位，在前面补0。
*/
string& ltime(string &strtime,const string &fmt="",const int timetvl=0);
char *    ltime(char *strtime   ,const string &fmt="",const int timetvl=0);
// 为了避免重载的岐义，增加ltime1()函数。
string    ltime1(const string &fmt="",const int timetvl=0);

// 把整数表示的时间转换为字符串表示的时间。
// ttime：整数表示的时间。
// strtime：字符串表示的时间。
// fmt：输出字符串时间strtime的格式，与ltime()函数的fmt参数相同，如果fmt的格式不正确，strtime将为空。
string& timetostr(const time_t ttime,string &strtime,const string &fmt="");
char*     timetostr(const time_t ttime,char *strtime   ,const string &fmt="");
// 为了避免重载的岐义，增加timetostr1()函数。
string    timetostr1(const time_t ttime,const string &fmt="");

// 把字符串表示的时间转换为整数表示的时间。
// strtime：字符串表示的时间，格式不限，但一定要包括yyyymmddhh24miss，一个都不能少，顺序也不能变。
// 返回值：整数表示的时间，如果strtime的格式不正确，返回-1。
time_t strtotime(const string &strtime);

// 把字符串表示的时间加上一个偏移的秒数后得到一个新的字符串表示的时间。
// in_stime：输入的字符串格式的时间，格式不限，但一定要包括yyyymmddhh24miss，一个都不能少，顺序也不能变。
// out_stime：输出的字符串格式的时间。
// timetvl：需要偏移的秒数，正数往后偏移，负数往前偏移。
// fmt：输出字符串时间out_stime的格式，与ltime()函数的fmt参数相同。
// 注意：in_stime和out_stime参数可以是同一个变量的地址，如果调用失败，out_stime的内容会清空。
// 返回值：true-成功，false-失败，如果返回失败，可以认为是in_stime的格式不正确。
bool addtime(const string &in_stime,char *out_stime    ,const int timetvl,const string &fmt="");
bool addtime(const string &in_stime,string &out_stime,const int timetvl,const string &fmt="");
///////////////////////////////////// /////////////////////////////////////

///////////////////////////////////// /////////////////////////////////////
// 这是一个精确到微秒的计时器。
class ctimer
{
private:
    struct timeval m_start;    // 计时开始的时间点。
    struct timeval m_end;     // 计时结束的时间点。
public:
    ctimer();          // 构造函数中会调用start方法。

    void start();     // 开始计时。

    // 计算已逝去的时间，单位：秒，小数点后面是微秒。
    // 每调用一次本方法之后，自动调用start方法重新开始计时。
    double elapsed();
};
///////////////////////////////////////////////////////////////////////////////////////////////////

// 根据绝对路径的文件名或目录名逐级的创建目录。
// pathorfilename：绝对路径的文件名或目录名。
// bisfilename：指定pathorfilename的类型，true-pathorfilename是文件名，否则是目录名，缺省值为true。
// 返回值：true-成功，false-失败，如果返回失败，原因有大概有三种情况：
// 1）权限不足；2）pathorfilename参数不是合法的文件名或目录名；3）磁盘空间不足。
bool newdir(const string &pathorfilename,bool bisfilename=true);

///////////////////////////////////// /////////////////////////////////////
// 文件操作相关的函数

// 重命名文件，类似Linux系统的mv命令。
// srcfilename：原文件名，建议采用绝对路径的文件名。
// dstfilename：目标文件名，建议采用绝对路径的文件名。
// 返回值：true-成功；false-失败，失败的主要原因是权限不足或磁盘空间不够，如果原文件和目标文件不在同一个磁盘分区，重命名也可能失败。
// 注意，在重命名文件之前，会自动创建dstfilename参数中包含的目录。
// 在应用开发中，可以用renamefile()函数代替rename()库函数。
bool renamefile(const string &srcfilename,const string &dstfilename);

// 复制文件，类似Linux系统的cp命令。
// srcfilename：原文件名，建议采用绝对路径的文件名。
// dstfilename：目标文件名，建议采用绝对路径的文件名。
// 返回值：true-成功；false-失败，失败的主要原因是权限不足或磁盘空间不够。
// 注意：
// 1）在复制文件之前，会自动创建dstfilename参数中的目录名。
// 2）复制文件的过程中，采用临时文件命名的方法，复制完成后再改名为dstfilename，避免中间状态的文件被读取。
// 3）复制后的文件的时间与原文件相同，这一点与Linux系统cp命令不同。
bool copyfile(const string &srcfilename,const string &dstfilename);

// 获取文件的大小。
// filename：待获取的文件名，建议采用绝对路径的文件名。
// 返回值：如果文件不存在或没有访问权限，返回-1，成功返回文件的大小，单位是字节。
int filesize(const string &filename);

// 获取文件的时间。
// filename：待获取的文件名，建议采用绝对路径的文件名。
// mtime：用于存放文件的时间，即stat结构体的st_mtime。
// fmt：设置时间的输出格式，与ltime()函数相同，但缺省是"yyyymmddhh24miss"。
// 返回值：如果文件不存在或没有访问权限，返回false，成功返回true。
bool filemtime(const string &filename,char *mtime    ,const string &fmt="yyyymmddhh24miss");
bool filemtime(const string &filename,string &mtime,const string &fmt="yyyymmddhh24miss");

// 重置文件的修改时间属性。
// filename：待重置的文件名，建议采用绝对路径的文件名。
// mtime：字符串表示的时间，格式不限，但一定要包括yyyymmddhh24miss，一个都不能少，顺序也不能变。
// 返回值：true-成功；false-失败，失败的原因保存在errno中。
bool setmtime(const string &filename,const string &mtime);
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////// /////////////////////////////////////
// 获取某目录及其子目录中的文件列表的类。
class cdir
{
private:
    vector<string> m_filelist;  // 存放文件列表的容器（绝对路径的文件名）。
    int m_pos;                          // 从文件列表m_filelist中已读取文件的位置。
    string m_fmt;                     // 文件时间格式，缺省"yyyymmddhh24miss"。

    cdir(const cdir &) = delete;                      // 禁用拷贝构造函数。
    cdir &operator=(const cdir &) = delete;  // 禁用赋值函数。
public:
    // /project/public/_public.h
    string m_dirname;        // 目录名，例如：/project/public
    string m_filename;       // 文件名，不包括目录名，例如：_public.h
    string m_ffilename;      // 绝对路径的文件，例如：/project/public/_public.h
    int      m_filesize;          // 文件的大小，单位：字节。
    string m_mtime;           // 文件最后一次被修改的时间，即stat结构体的st_mtime成员。
    string m_ctime;            // 文件生成的时间，即stat结构体的st_ctime成员。
    string m_atime;            // 文件最后一次被访问的时间，即stat结构体的st_atime成员。

    cdir():m_pos(0),m_fmt("yyyymmddhh24miss") {}  // 构造函数。

    // 设置文件时间的格式，支持"yyyy-mm-dd hh24:mi:ss"和"yyyymmddhh24miss"两种，缺省是后者。
    void setfmt(const string &fmt);

    // 打开目录，获取目录中文件的列表，存放在m_filelist容器中。
    // dirname，目录名，采用绝对路径，如/tmp/root。
    // rules，文件名的匹配规则，不匹配的文件将被忽略。
    // maxfiles，本次获取文件的最大数量，缺省值为10000个，如果文件太多，可能消耗太多的内存。
    // bandchild，是否打开各级子目录，缺省值为false-不打开子目录。
    // bsort，是否按文件名排序，缺省值为false-不排序。
    // 返回值：true-成功，false-失败。
    bool opendir(const string &dirname,const string &rules,const int maxfiles=10000,const bool bandchild=false,bool bsort=false);

private:
    // 这是一个递归函数，被opendir()的调用，在cdir类的外部不需要调用它。
    bool _opendir(const string &dirname,const string &rules,const int maxfiles,const bool bandchild);

public:
    // 从m_filelist容器中获取一条记录（文件名），同时获取该文件的大小、修改时间等信息。
    // 调用opendir方法时，m_filelist容器被清空，m_pos归零，每调用一次readdir方法m_pos加1。
    // 当m_pos小于m_filelist.size()，返回true，否则返回false。
    bool readdir();

    unsigned int size() { return m_filelist.size(); }

    ~cdir();  // 析构函数。
};
///////////////////////////////////// /////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// 写文件的类。
class cofile  // class out file
{
private:
    ofstream fout;                  // 写入文件的对象。
    string   m_filename;         // 文件名，建议采用绝对路径。
    string   m_filenametmp;  // 临时文件名，在m_filename后面加".tmp"。
public:
    cofile() {}
    bool isopen() const { return fout.is_open(); }     // 文件是否已打开。

    // 打开文件。
    // filename，待打开的文件名。
    // btmp，是否采用临时文件的方案。
    // mode，打开文件的模式。
    // benbuffer，是否启用文件缓冲区。
     bool open(const string &filename,const bool btmp=true,const ios::openmode mode=ios::out,const bool benbuffer=true);

    // 把数据以文本的方式格式化输出到文件。
    template< typename... Args >
    bool writeline(const char* fmt, Args... args) 
    {
        if (fout.is_open()==false) return false;

        fout << sformat(fmt,args...);

        return fout.good();
    }

    // 重载<<运算符，把数据以文本的方式输出到文件。
    // 注意：换行只能用\n，不能用endl。
    template<typename T>
    cofile& operator<<(const T &value)
    {
        fout << value; return *this;
    }

    // 把二进制数据写入文件。
    bool write(void *buf,int bufsize);

    // 关闭文件，并且把临时文件名改为正式文件名。
    bool closeandrename();

    // 关闭文件，如果有临时文件，则删除它。
    void close();

    ~cofile() { close(); };
};
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// 读取文件的类。
class cifile    // class in file
{
private:
    ifstream fin;                     // 读取文件的对象。
    string   m_filename;         // 文件名，建议采用绝对路径。
public:
    cifile() {}

    // 判断文件是否已打开。
    bool isopen() const { return fin.is_open(); }

    // 打开文件。
    // filename，待打开的文件名。
    // mode，打开文件的模式。
    bool open(const string &filename,const ios::openmode mode=ios::in);

    // 以行的方式读取文本文件，endbz指定行的结尾标志，缺省为空，没有结尾标志。
    bool readline(string &buf,const string& endbz="");

    // 读取二进制文件，返回实际读取到的字节数。
    int read(void *buf,const int bufsize);

    // 关闭并删除文件。
    bool closeandremove();

    // 只关闭文件。
    void close();

    ~cifile() { close(); }
};
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// 自旋锁。
class spinlock_mutex
{
private:
    atomic_flag flag;

    spinlock_mutex(const spinlock_mutex&) = delete;
    spinlock_mutex& operator=(const spinlock_mutex) = delete;
public:
    spinlock_mutex() 
    {
        flag.clear();
    }
    void lock()           // 加锁。
    {
        while (flag.test_and_set())
            ;
    }
    void unlock()      // 解锁。
    {
        flag.clear();
    }
};

///////////////////////////////////// /////////////////////////////////////
// 日志文件。
class clogfile
{
    ofstream fout;                       // 日志文件对象。
    string   m_filename;              // 日志文件名，建议采用绝对路径。
    ios::openmode m_mode;      // 日志文件的打开模式。
    bool     m_backup;                // 是否自动切换日志。
    int        m_maxsize;               // 当日志文件的大小超过本参数时，自动切换日志。
    bool     m_enbuffer;              // 是否启用文件缓冲区。
    spinlock_mutex m_splock;    // 自旋锁，用于多线程程序中给写日志的操作加锁。

public:
    // 构造函数，日志文件的大小缺省100M。
    clogfile(int maxsize=100):m_maxsize(maxsize){}

    // 打开日志文件。
    // filename：日志文件名，建议采用绝对路径，如果文件名中的目录不存在，就先创建目录。
    // openmode：日志文件的打开模式，缺省值是ios::app。
    // bbackup：是否自动切换（备份），true-切换，false-不切换，在多进程的服务程序中，如果多个进程共用一个日志文件，bbackup必须为false。
    // benbuffer：是否启用文件缓冲机制，true-启用，false-不启用，如果启用缓冲区，那么写进日志文件中的内容不会立即写入文件，缺省是不启用。
    // 注意，在多进程的程序中，多个进程往同一日志文件写入大量的日志时，可能会出现小混乱，但是，多线程不会。
    // 1）多个进程往同一日志文件写入大量的日志时，可能会出现小混乱，这个问题并不严重，可以容忍；
    // 2）只有同时写大量日志时才会出现混乱，在实际开发中，这种情况不多见。
    // 3）如果业务无法容忍，可以用信号量加锁。
    bool open(const string &filename,const ios::openmode mode=ios::app,const bool bbackup=true,const bool benbuffer=false);

    // 把日志内容以文本的方式格式化输出到日志文件，并且，在日志内容前面写入时间。
    template< typename... Args >
    bool write(const char* fmt, Args... args) 
    {
        if (fout.is_open()==false) return false;

        backup();                   // 判断是否需要切换日志文件。

        m_splock.lock();        // 加锁。
        fout << ltime1() << " " << sformat(fmt,args...);      // 把当前时间和日志内容写入日志文件。
        m_splock.unlock();    // 解锁。

        return fout.good();
    }

    // 重载<<运算符，把日志内容以文本的方式输出到日志文件，不会在日志内容前面写时间。
    // 注意：内容换行用\n，不能用endl。
    template<typename T>
    clogfile& operator<<(const T &value)
    {
        m_splock.lock();
        fout << value; 
        m_splock.unlock();

        return *this;
    }
  
private:
    // 如果日志文件的大小超过m_maxsize的值，就把当前的日志文件名改为历史日志文件名，再创建新的当前日志文件。
    // 备份后的文件会在日志文件名后加上日期时间，如/tmp/log/filetodb.log.20200101123025。
    // 注意，在多进程的程序中，日志文件不可切换，多线的程序中，日志文件可以切换。
    bool backup();
public:
    void close() { fout.close(); }

    ~clogfile() { close(); };
};
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// 以下是socket通讯的函数和类

// socket通讯的客户端类
class ctcpclient
{
private:
    int  m_connfd;    // 客户端的socket.
    string m_ip;        // 服务端的ip地址。
    int  m_port;        // 服务端通讯的端口。
public:
    ctcpclient(): m_connfd(-1),m_port(0) { }  // 构造函数。

    // 向服务端发起连接请求。
    // ip：服务端的ip地址。
    // port：服务端通讯的端口。
    // 返回值：true-成功；false-失败。
    bool connect(const string &ip,const int port);

    // 接收对端发送过来的数据。
    // buffer：存放接收数据缓冲区。
    // ibuflen: 打算接收数据的大小。
    // itimeout：等待数据的超时时间（秒）：-1-不等待；0-无限等待；>0-等待的秒数。
    // 返回值：true-成功；false-失败，失败有两种情况：1）等待超时；2）socket连接已不可用。
    bool read(string &buffer,const int itimeout=0);                           // 接收文本数据。
    bool read(void *buffer,const int ibuflen,const int itimeout=0);   // 接收二进制数据。

    // 向对端发送数据。
    // buffer：待发送数据缓冲区。
    // ibuflen：待发送数据的大小。
    // 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
    bool write(const string &buffer);                          // 发送文本数据。
    bool write(const void *buffer,const int ibuflen);   // 发送二进制数据。

    // 断开与服务端的连接
    void close();

    ~ctcpclient();  // 析构函数自动关闭socket，释放资源。
};

// socket通讯的服务端类
class ctcpserver
{
private:
    int m_socklen;                                // 结构体struct sockaddr_in的大小。
    struct sockaddr_in m_clientaddr;   // 客户端的地址信息。
    struct sockaddr_in m_servaddr;     // 服务端的地址信息。
    int  m_listenfd;                               // 服务端用于监听的socket。
    int  m_connfd;                                // 客户端连接上来的socket。
public:
    ctcpserver():m_listenfd(-1),m_connfd(-1) {}  // 构造函数。

    // 服务端初始化。
    // port：指定服务端用于监听的端口。
    // 返回值：true-成功；false-失败，一般情况下，只要port设置正确，没有被占用，初始化都会成功。
    bool initserver(const unsigned int port,const int backlog=5); 

    // 从已连接队列中获取一个客户端连接，如果已连接队列为空，将阻塞等待。
    // 返回值：true-成功的获取了一个客户端连接，false-失败，如果accept失败，可以重新accept。
    bool accept();

    // 获取客户端的ip地址。
    // 返回值：客户端的ip地址，如"192.168.1.100"。
    char *getip();

    // 接收对端发送过来的数据。
    // buffer：存放接收数据的缓冲区。
    // ibuflen: 打算接收数据的大小。
    // itimeout：等待数据的超时时间（秒）：-1-不等待；0-无限等待；>0-等待的秒数。
    // 返回值：true-成功；false-失败，失败有两种情况：1）等待超时；2）socket连接已不可用。
    bool read(string &buffer,const int itimeout=0);                           // 接收文本数据。
    bool read(void *buffer,const int ibuflen,const int itimeout=0);   // 接收二进制数据。

    // 向对端发送数据。
    // buffer：待发送数据缓冲区。
    // ibuflen：待发送数据的大小。
    // 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
    bool write(const string &buffer);                          // 发送文本数据。
    bool write(const void *buffer,const int ibuflen);   // 发送二进制数据。

    // 关闭监听的socket，即m_listenfd，常用于多进程服务程序的子进程代码中。
    void closelisten();

    // 关闭客户端的socket，即m_connfd，常用于多进程服务程序的父进程代码中。
    void closeclient();

    ~ctcpserver();  // 析构函数自动关闭socket，释放资源。
};

// 接收socket的对端发送过来的数据。
// sockfd：可用的socket连接。
// buffer：接收数据缓冲区的地址。
// ibuflen：本次成功接收数据的字节数。
// itimeout：读取数据超时的时间，单位：秒，-1-不等待；0-无限等待；>0-等待的秒数。
// 返回值：true-成功；false-失败，失败有两种情况：1）等待超时；2）socket连接已不可用。
bool tcpread(const int sockfd,string &buffer,const int itimeout=0);                            // 读取文本数据。
bool tcpread(const int sockfd,void *buffer,const int ibuflen,const int itimeout=0);     // 读取二进制数据。

// 向socket的对端发送数据。
// sockfd：可用的socket连接。
// buffer：待发送数据缓冲区的地址。
// ibuflen：待发送数据的字节数。
// 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
bool tcpwrite(const int sockfd,const string &buffer);                             // 写入文本数据。
bool tcpwrite(const int sockfd,const void *buffer,const int ibuflen);      // 写入二进制数据。

// 从已经准备好的socket中读取数据。
// sockfd：已经准备好的socket连接。
// buffer：存放数据的地址。
// n：本次打算读取数据的字节数。
// 返回值：成功接收到n字节的数据后返回true，socket连接不可用返回false。
bool readn(const int sockfd,char *buffer,const size_t n);

// 向已经准备好的socket中写入数据。
// sockfd：已经准备好的socket连接。
// buffer：待写入数据的地址。
// n：待写入数据的字节数。
// 返回值：成功写入完n字节的数据后返回true，socket连接不可用返回false。
bool writen(const int sockfd,const char *buffer,const size_t n);

// 以上是socket通讯的函数和类
///////////////////////////////////// /////////////////////////////////////

// 忽略关闭全部的信号、关闭全部的IO，缺省只忽略信号，不关IO。
void closeioandsignal(bool bcloseio=false);

// 循环队列。
template <class TT, int MaxLength>
class squeue
{
private:
    bool m_inited;              // 队列被初始化标志，true-已初始化；false-未初始化。
    TT   m_data[MaxLength];     // 用数组存储循环队列中的元素。
    int  m_head;                // 队列的头指针。
    int  m_tail;                // 队列的尾指针，指向队尾元素。
    int  m_length;              // 队列的实际长度。    
    squeue(const squeue &) = delete;             // 禁用拷贝构造函数。
    squeue &operator=(const squeue &) = delete;  // 禁用赋值函数。
public:

    squeue() { init(); }  // 构造函数。

    // 循环队列的初始化操作。
    // 注意：如果用于共享内存的队列，不会调用构造函数，必须调用此函数初始化。
    void init()  
    { 
        if (m_inited!=true)               // 循环队列的初始化只能执行一次。
        { 
            m_head=0;                      // 头指针。
            m_tail=MaxLength-1;     // 为了方便写代码，初始化时，尾指针指向队列的最后一个位置。
            m_length=0;                   // 队列的实际长度。
            memset(m_data,0,sizeof(m_data));  // 数组元素清零。
            m_inited=true; 
        }
    }

    // 元素入队，返回值：false-失败；true-成功。
    bool push(const TT &ee)
    {
        if (full() == true)
        {
            cout << "循环队列已满，入队失败。\n"; return false;
        }

        // 先移动队尾指针，然后再拷贝数据。
        m_tail=(m_tail+1)%MaxLength;  // 队尾指针后移。
        m_data[m_tail]=ee;
        m_length++;    

      return true;
    }

    // 求循环队列的长度，返回值：>=0-队列中元素的个数。
    int  size()                   
    {
        return m_length;    
    }

    // 判断循环队列是否为空，返回值：true-空，false-非空。
    bool empty()                    
    {
      if (m_length == 0) return true;    

      return false;
    }

    // 判断循环队列是否已满，返回值：true-已满，false-未满。
    bool full()
    {
        if (m_length == MaxLength) return true;    

      return false;
    }

    // 查看队头元素的值，元素不出队。
    TT& front()
    {
        return m_data[m_head];
    }

    // 元素出队，返回值：false-失败；true-成功。
    bool pop()
    {
        if (empty() == true) return false;

        m_head=(m_head+1)%MaxLength;  // 队列头指针后移。
        m_length--;    

        return true;
    }

    // 显示循环队列中全部的元素。
    // 这是一个临时的用于调试的函数，队列中元素的数据类型支持cout输出才可用。
    void printqueue()                    
    {
        for (int ii = 0; ii < size(); ii++)
        {
            cout << "m_data[" << (m_head+ii)%MaxLength << "],value=" \
                 << m_data[(m_head+ii)%MaxLength] << endl;
        }
    }
};

// 信号量。
class csemp
{
private:
    union semun  // 用于信号量操作的共同体。
    {
      int val;
      struct semid_ds *buf;
      unsigned short  *arry;
    };

    int   m_semid;         // 信号量id（描述符）。

    // 如果把sem_flg设置为SEM_UNDO，操作系统将跟踪进程对信号量的修改情况，
    // 在全部修改过信号量的进程（正常或异常）终止后，操作系统将把信号量恢复为初始值。
    // 如果信号量用于互斥锁，设置为SEM_UNDO。
    // 如果信号量用于生产消费者模型，设置为0。
    short m_sem_flg;

    csemp(const csemp &) = delete;                      // 禁用拷贝构造函数。
    csemp &operator=(const csemp &) = delete;  // 禁用赋值函数。
public:
    csemp():m_semid(-1){}

    // 如果信号量已存在，获取信号量；如果信号量不存在，则创建它并初始化为value。
    // 如果用于互斥锁，value填1，sem_flg填SEM_UNDO。
    // 如果用于生产消费者模型，value填0，sem_flg填0。
    bool init(key_t key,unsigned short value=1,short sem_flg=SEM_UNDO);
    bool wait(short value=-1);    // 信号量的P操作，如果信号量的值是0，将阻塞等待，直到信号量的值大于0。
    bool post(short value=1);     // 信号量的V操作。
    int  getvalue();                       // 获取信号量的值，成功返回信号量的值，失败返回-1。
    bool destroy();                       // 销毁信号量。
    ~csemp();
};

// 进程心跳信息的结构体。
struct st_procinfo
{
    int      pid=0;                      // 进程id。
    char   pname[51]={0};        // 进程名称，可以为空。
    int      timeout=0;              // 超时时间，单位：秒。
    time_t atime=0;                 // 最后一次心跳的时间，用整数表示。
    st_procinfo() = default;     // 有了自定义的构造函数，编译器将不提供默认构造函数，所以启用默认构造函数。
    st_procinfo(const int in_pid,const string & in_pname,const int in_timeout, const time_t in_atime)
                    :pid(in_pid),timeout(in_timeout),atime(in_atime) { strncpy(pname,in_pname.c_str(),50); }
};

// 以下几个宏用于进程的心跳。
#define MAXNUMP     1000     // 最大的进程数量。
#define SHMKEYP    0x5095     // 共享内存的key。
#define SEMKEYP     0x5095     // 信号量的key。

// 查看共享内存：  ipcs -m
// 删除共享内存：  ipcrm -m shmid
// 查看信号量：      ipcs -s
// 删除信号量：      ipcrm sem semid

// 进程心跳操作类。
class cpactive
{
 private:
     int  m_shmid;                   // 共享内存的id。
     int  m_pos;                       // 当前进程在共享内存进程组中的位置。
     st_procinfo *m_shm;        // 指向共享内存的地址空间。

 public:
     cpactive();  // 初始化成员变量。

     // 把当前进程的信息加入共享内存进程组中。
     bool addpinfo(const int timeout,const string &pname="",clogfile *logfile=nullptr);

     // 更新共享内存进程组中当前进程的心跳时间。
     bool uptatime();

     ~cpactive();  // 从共享内存中删除当前进程的心跳记录。
};

}
#endif
