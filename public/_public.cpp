/*****************************************************************************************/
/*   程序名：_public.cpp，此程序是公共函数和类的定义文件。                       */
/*   作者：吴从周                                                                        */
/*****************************************************************************************/

#include "_public.h"  

namespace idc
{

char *deletelchr(char* str, const int cc)
{
    if (str == nullptr) return nullptr;		// 如果传进来的是空地址，直接返回，防止程序崩溃。
	
    char* p = str;				   // 指向字符串的首地址。
    while (*p == cc)			// 遍历字符串，p将指向左边第一个不是cc的字符。
        p++;		

    memmove(str, p, strlen(str) - (p - str)+1);  // 把结尾标志0也拷过来。

    return str;
}

string& deletelchr(string &str, const int cc)
{
    auto pos=str.find_first_not_of(cc);    // 从字符串的左边查找第一个不是cc的字符的位置。

    if (pos!= 0) str.replace(0,pos,"");       // 把0-pos之间的字符串替换成空。

    return str;
}

char* deleterchr(char *str,const int cc)
{
    if (str == nullptr) return nullptr;	// 如果传进来的是空地址，直接返回，防止程序崩溃。
	
    char* p = str;				// 指向字符串的首地址。
    char* piscc = 0;		  // 右边全是字符cc的第一个位置。

    while (*p != 0)			   // 遍历字符串。
    {
        if (*p == cc && piscc == 0) piscc = p;		  // 记下字符cc的第一个位置。
        if (*p != cc) piscc = 0;								  // 只要当前字符不是cc，清空piscc。
        p++;		
	  }

	  if (piscc != 0) *piscc = 0;	// 把piscc位置的字符置为0，表示字符串已结束。

    return str;
}

string& deleterchr(string &str,const int cc)
{
    auto pos=str.find_last_not_of(cc);     // 从字符串的右边查找第一个不是cc的字符的位置。

    if (pos!= 0) str.erase(pos+1);            // 把pos之后的字符删掉。

    return str;
}

char* deletelrchr(char *str,const int cc)
{
    deletelchr(str,cc);
    deleterchr(str,cc);

    return str;
}

string& deletelrchr(string &str,const int cc)
{
    deletelchr(str,cc);
    deleterchr(str,cc);

    return str;
}

char* toupper(char *str)
{
    if (str == nullptr) return nullptr;

    char* p = str;				// 指向字符串的首地址。
    while (*p != 0)			  // 遍历字符串。
    {
        if ( (*p >= 'a') && (*p <= 'z') ) *p=*p - 32;
        p++;
    }

    return str;
}

string& toupper(string &str)
{
    for (auto &cc:str)
    {
        if ( (cc >= 'a') && (cc <= 'z') ) cc=cc - 32;
    }

    return str;
}

char* tolower(char *str)
{
    if (str == nullptr) return nullptr;

    char* p = str;				// 指向字符串的首地址。
    while (*p != 0)			  // 遍历字符串。
    {
        if ( (*p >= 'A') && (*p <= 'Z') ) *p=*p + 32;
        p++;
    }

    return str;
}

string& tolower(string &str)
{
    for (auto &cc:str)
    {
        if ( (cc >= 'A') && (cc <= 'Z') ) cc=cc + 32;
    }

    return str;
}

bool replacestr(string &str,const string &str1,const string &str2,bool bloop)
{
    // 如果原字符串str或旧的内容str1为空，没有意义，不执行替换。
    if ( (str.length() == 0) || (str1.length() == 0) ) return false;
  
    // 如果bloop为true并且str2中包函了str1的内容，直接返回，因为会进入死循环，最终导致内存溢出。
    if ( (bloop==true) && (str2.find(str1)!=string::npos) ) return false;

    int pstart=0;      // 如果bloop==false，下一次执行替换的开始位置。
    int ppos=0;        // 本次需要替换的位置。

    while (true)
    {
        if (bloop == true)
            ppos=str.find(str1);                      // 每次从字符串的最左边开始查找子串str1。
        else
            ppos=str.find(str1,pstart);            // 从上次执行替换的位置后开始查找子串str1。

        if (ppos == string::npos) break;       // 如果没有找到子串str1。

        str.replace(ppos,str1.length(),str2);   // 把str1替换成str2。

        if (bloop == false) pstart=ppos+str2.length();    // 下一次执行替换的开始位置往右移动。
    }

    return true;
}

bool replacestr(char *str,const string &str1,const string &str2,bool bloop)
{
    if (str == nullptr) return false;
  
    string strtemp(str);

    replacestr(strtemp,str1,str2,bloop);

    strtemp.copy(str,strtemp.length());
    str[strtemp.length()]=0;    // string的copy函数不会给C风格字符串的结尾加0。

    return true;
}

char* picknumber(const string &src,char *dest,const bool bsigned,const bool bdot)
{
    if (dest==nullptr) return nullptr;    // 判断空指针。

    string strtemp=picknumber(src,bsigned,bdot);
    strtemp.copy(dest,strtemp.length());
    dest[strtemp.length()]=0;    // string的copy函数不会给C风格字符串的结尾加0。

    return dest;
}

string& picknumber(const string &src,string &dest,const bool bsigned,const bool bdot)
{
    // 为了支持src和dest是同一变量的情况，定义str临时变量。
    string str;

    for (char cc:src)
    {
        // 判断是否提取符号。
        if ( (bsigned==true) && ( (cc == '+') || (cc == '-') ))
        {
            str.append(1,cc); continue;
        }

        // 判断是否提取小数点。
        if ( (bdot==true) && (cc == '.') )
        {
            str.append(1,cc); continue;
        }

        // 提取数字。
        if (isdigit(cc)) str.append(1,cc);
    }

    dest=str;

    return dest;
}

string picknumber(const string &src,const bool bsigned,const bool bdot)
{
    string dest;
    picknumber(src,dest,bsigned,bdot);
    return dest;
}

bool matchstr(const string &str,const string &rules)
{
    // 如果匹配规则表达式的内容是空的，返回false。
    if (rules.length() == 0) return false;

    // 如果如果匹配规则表达式的内容是"*"，直接返回true。
    if (rules == "*") return true;

    int  ii,jj;
    int  pos1,pos2;
    ccmdstr cmdstr,cmdsubstr;

    string filename=str;
    string matchstr=rules;

    // 把字符串都转换成大写后再来比较
    toupper(filename);
    toupper(matchstr);

    cmdstr.splittocmd(matchstr,",");

    for (ii=0;ii<cmdstr.size();ii++)
    {
        // 如果为空，就一定要跳过，否则就会被匹配上。
        if (cmdstr[ii].empty() == true) continue;

        pos1=pos2=0;
        cmdsubstr.splittocmd(cmdstr[ii],"*");

        for (jj=0;jj<cmdsubstr.size();jj++)
        {
            // 如果是文件名的首部
            if (jj == 0)
                if (filename.substr(0,cmdsubstr[jj].length())!=cmdsubstr[jj]) break;

            // 如果是文件名的尾部
            if (jj == cmdsubstr.size()-1)
                if (filename.find(cmdsubstr[jj],filename.length()-cmdsubstr[jj].length()) == string::npos) break;

            pos2=filename.find(cmdsubstr[jj],pos1);

            if (pos2 == string::npos) break;

            pos1=pos2+cmdsubstr[jj].length();
        }

        if (jj==cmdsubstr.size()) return true;
    }

    return false;
}

ccmdstr::ccmdstr(const string &buffer,const string &sepstr,const bool bdelspace)
{
    splittocmd(buffer,sepstr,bdelspace);
}

// 把字符串拆分到m_cmdstr容器中。
// buffer：待拆分的字符串。
// sepstr：buffer字符串中字段内容的分隔符，注意，分隔符是字符串，如","、" "、"|"、"~!~"。
// bdelspace：是否删除拆分后的字段内容前后的空格，true-删除；false-不删除，缺省不删除。
void ccmdstr::splittocmd(const string &buffer,const string &sepstr,const bool bdelspace)
{
    // 清除所有的旧数据
    m_cmdstr.clear();

    int pos=0;        // 每次从buffer中查找分隔符的起始位置。
    int pos1=0;      // 从pos的位置开始，查找下一个分隔符的位置。
    string substr;   // 存放每次拆分出来的子串。

    while ( (pos1=buffer.find(sepstr,pos)) != string::npos)   // 从pos的位置开始，查找下一个分隔符的位置。
    {
        substr=buffer.substr(pos,pos1-pos);            // 从buffer中截取子串。

        if (bdelspace == true) deletelrchr(substr);   // 删除子串前后的空格。

        m_cmdstr.push_back(std::move(substr));     // 把子串放入m_cmdstr容器中，调用string类的移动构造函数。

        pos=pos1+sepstr.length();                           // 下次从buffer中查找分隔符的起始位置后移。
    }

    // 处理最后一个字段（最后一个分隔符之后的内容）。
    substr=buffer.substr(pos);

    if (bdelspace == true) deletelrchr(substr);

    m_cmdstr.push_back(std::move(substr));

    return;
}

bool ccmdstr::getvalue(const int ii,string &value,const int ilen) const
{
    if (ii>=m_cmdstr.size()) return false;

    // 从xml中截取数据项的内容。
    // 视频中是以下代码：
    // value=m_cmdstr[ii];
    // 改为：
    int itmplen=m_cmdstr[ii].length();
    if ( (ilen>0) && (ilen<itmplen) ) itmplen=ilen;
    value=m_cmdstr[ii].substr(0,itmplen);
  
    return true;
}

bool ccmdstr::getvalue(const int ii,char *value,const int len) const
{
    if ( (ii>=m_cmdstr.size()) || (value==nullptr) ) return false;

    if (len>0) memset(value,0,len+1);   // 调用者必须保证value的空间足够，否则这里会内存溢出。

    if ( (m_cmdstr[ii].length()<=(unsigned int)len) || (len==0) )
    {
        m_cmdstr[ii].copy(value,m_cmdstr[ii].length());
        value[m_cmdstr[ii].length()]=0;    // string的copy函数不会给C风格字符串的结尾加0。
    }
    else
    {
        m_cmdstr[ii].copy(value,len);
        value[len]=0;
    }

    return true;
}

bool ccmdstr::getvalue(const int ii,int &value) const
{
    if (ii>=m_cmdstr.size()) return false;

    try
    {
        value = stoi(picknumber(m_cmdstr[ii],true));  // stoi有异常，需要处理异常。
    }
    catch(const std::exception& e)
    {
        return false;
    }
    
    return true;
}

bool ccmdstr::getvalue(const int ii,unsigned int &value) const
{
    if (ii>=m_cmdstr.size()) return false;

    try
    {
       value = stoi(picknumber(m_cmdstr[ii]));  // stoi有异常，需要处理异常。不提取符号 + -
    }
    catch(const std::exception& e)
    {
        return false;
    }

    return true;
}

bool ccmdstr::getvalue(const int ii,long &value) const
{
    if (ii>=m_cmdstr.size()) return false;

    try
    {
        value = stol(picknumber(m_cmdstr[ii],true));  // stol有异常，需要处理异常。
    }
    catch(const std::exception& e)
    {
        return false;
    }

    return true;
}

bool ccmdstr::getvalue(const int ii,unsigned long &value) const
{
    if (ii>=m_cmdstr.size()) return false;

    try
    {
        value = stoul(picknumber(m_cmdstr[ii]));  // stoul有异常，需要处理异常。不提取符号 + -
    }
    catch(const std::exception& e)
    {
        return false;
    }

    return true;
}

bool ccmdstr::getvalue(const int ii,double &value) const
{
    if (ii>=m_cmdstr.size()) return false;

    try
    {
        value = stod(picknumber(m_cmdstr[ii],true,true));  // stod有异常，需要处理异常。提取符号和小数点。
    }
    catch(const std::exception& e)
    {
        return false;
    }

    return true;
}

bool ccmdstr::getvalue(const int ii,float &value) const
{
    if (ii>=m_cmdstr.size()) return false;

    try
    {
        value = stof(picknumber(m_cmdstr[ii],true,true));  // stof有异常，需要处理异常。提取符号和小数点。
    }
    catch(const std::exception& e)
    {
        return false;
    }

    return true;
}

bool ccmdstr::getvalue(const int ii,bool &value) const
{
    if (ii>=m_cmdstr.size()) return false;

    string str=m_cmdstr[ii];
    toupper(str);     // 转换为大写来判断。

    if (str=="TRUE") value=true; 
    else value=false;

    return true;
}

ccmdstr::~ccmdstr()
{
    m_cmdstr.clear();
}

ostream& operator<<(ostream& out, const ccmdstr& cmdstr)
{
    for (int ii=0;ii<cmdstr.size();ii++)
        out << "[" << ii << "]=" << cmdstr[ii] << endl;

    return out;
}

bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,string  &value,const int ilen)
{
    string start="<"+fieldname+">";            // 数据项开始的标签。
    string end="</"+fieldname+">";            // 数据项结束的标签。

    int startp=xmlbuffer.find(start);               // 在xml中查找数据项开始的标签的位置。
    if (startp==string::npos) return false;

    int endp=xmlbuffer.find(end);                 // 在xml中查找数据项结束的标签的位置。
    if (endp==string::npos) return false;

    // 从xml中截取数据项的内容。
    // 视频中是以下代码：
    // value=xmlbuffer.substr(startp+start.length(),endp-startp-start.length());
    // 改为：
    int itmplen=endp-startp-start.length();
    if ( (ilen>0) && (ilen<itmplen) ) itmplen=ilen;
    value=xmlbuffer.substr(startp+start.length(),itmplen);

    return true;
}

bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,char *value,const int len)
{
    if (value==nullptr) return false;

    if (len>0) memset(value,0,len+1);   // 调用者必须保证value的空间足够，否则这里会内存溢出。

    string str;
    getxmlbuffer(xmlbuffer,fieldname,str);

    if ( (str.length()<=(unsigned int)len) || (len==0) )
    {
        str.copy(value,str.length());
        value[str.length()]=0;    // string的copy函数不会给C风格字符串的结尾加0。
    }
    else
    {
        str.copy(value,len);
        value[len]=0;
    }

    return true;
}

bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,bool &value)
{
    string str;
    if (getxmlbuffer(xmlbuffer,fieldname,str)==false) return false;

    toupper(str);    // 转换为大写来判断（也可以转换为小写，效果相同）。

    if (str=="TRUE") value=true; 
    else value=false;

    return true;
}

bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,int &value)
{
    string str;

    if (getxmlbuffer(xmlbuffer,fieldname,str)==false) return false;

    try
    {
       value = stoi(picknumber(str,true));  // stoi有异常，需要处理异常。
    }
    catch(const std::exception& e)
    {
        return false;
    }

    return true;
}

bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,unsigned int &value)
{
    string str;

    if (getxmlbuffer(xmlbuffer,fieldname,str)==false) return false;

    try
    {
       value = stoi(picknumber(str));  // stoi有异常，需要处理异常。不提取符号 + -
    }
    catch(const std::exception& e)
    {
        return false;
    }

    return true;
}

bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,long &value)
{
    string str;

    if (getxmlbuffer(xmlbuffer,fieldname,str)==false) return false;

    try
    {
        value = stol(picknumber(str,true));  // stol有异常，需要处理异常。
    }
    catch(const std::exception& e)
    {
        return false;
    }

    return true;
}

bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,unsigned long &value)
{
    string str;

    if (getxmlbuffer(xmlbuffer,fieldname,str)==false) return false;

    try
    {
        value = stoul(picknumber(str));  // stoul有异常，需要处理异常。不提取符号 + -
    }
    catch(const std::exception& e)
    {
        return false;
    }

    return true;
}

bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,double &value)
{
    string str;

    if (getxmlbuffer(xmlbuffer,fieldname,str)==false) return false;

    try
    {
        value = stod(picknumber(str,true,true));  // stod有异常，需要处理异常。提取符号和小数点。
    }
    catch(const std::exception& e)
    {
        return false;
    }

    return true;
}

bool getxmlbuffer(const string &xmlbuffer,const string &fieldname,float &value)
{
    string str;

    if (getxmlbuffer(xmlbuffer,fieldname,str)==false) return false;

    try
    {
        value = stof(picknumber(str,true,true));  // stof有异常，需要处理异常。提取符号和小数点。
    }
    catch(const std::exception& e)
    {
        return false;
    }

    return true;
}

// 把整数表示的时间转换为字符串表示的时间。
// ttime：整数表示的时间。
// strtime：字符串表示的时间。
// fmt：输出字符串时间strtime的格式，与ttime函数的fmt参数相同，如果fmt的格式不正确，strtime将为空。
string& timetostr(const time_t ttime,string &strtime,const string &fmt)
{
    //struct tm sttm = *localtime ( &ttime );        // 非线程安全。
    struct tm sttm; localtime_r (&ttime,&sttm);   // 线程安全。
    sttm.tm_year=sttm.tm_year+1900;                // tm.tm_year成员要加上1900。
    sttm.tm_mon++;                                            // sttm.tm_mon成员是从0开始的，要加1。

    // 缺省的时间格式。
    if ( (fmt=="") || (fmt=="yyyy-mm-dd hh24:mi:ss") )
    {
        strtime=sformat("%04u-%02u-%02u %02u:%02u:%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday,\
                   sttm.tm_hour,sttm.tm_min,sttm.tm_sec);
        return strtime;
    }

    if (fmt=="yyyy-mm-dd hh24:mi")
    {
        strtime=sformat("%04u-%02u-%02u %02u:%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday,\
                   sttm.tm_hour,sttm.tm_min);
        return strtime;
    }

    if (fmt=="yyyy-mm-dd hh24")
    {
        strtime=sformat("%04u-%02u-%02u %02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday,sttm.tm_hour);
        return strtime;
    }

    if (fmt=="yyyy-mm-dd")
    {
        strtime=sformat("%04u-%02u-%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday); 
        return strtime;
    }

    if (fmt=="yyyy-mm")
    {
        strtime=sformat("%04u-%02u",sttm.tm_year,sttm.tm_mon); 
        return strtime;
    }

    if (fmt=="yyyymmddhh24miss") 
    {
        strtime=sformat("%04u%02u%02u%02u%02u%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday,\
                   sttm.tm_hour,sttm.tm_min,sttm.tm_sec);
        return strtime;
    }

    if (fmt=="yyyymmddhh24mi")
    {
        strtime=sformat("%04u%02u%02u%02u%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday,\
                   sttm.tm_hour,sttm.tm_min);
        return strtime;
    }

    if (fmt=="yyyymmddhh24")
    {
        strtime=sformat("%04u%02u%02u%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday,sttm.tm_hour);
        return strtime;
    }

    if (fmt=="yyyymmdd")
    {
        strtime=sformat("%04u%02u%02u",sttm.tm_year,sttm.tm_mon,sttm.tm_mday); 
        return strtime;
    }

    if (fmt=="hh24miss")
    {
        strtime=sformat("%02u%02u%02u",sttm.tm_hour,sttm.tm_min,sttm.tm_sec); 
        return strtime;
    }

    if (fmt=="hh24mi") 
    {
        strtime=sformat("%02u%02u",sttm.tm_hour,sttm.tm_min); 
        return strtime;
    }

    if (fmt=="hh24")
    {
        strtime=sformat("%02u",sttm.tm_hour); 
        return strtime;
    }

    if (fmt=="mi")
    {
        strtime=sformat("%02u",sttm.tm_min); 
        return strtime;
    }

    return strtime;
}

char* timetostr(const time_t ttime,char *strtime,const string &fmt)
{
    if (strtime==nullptr) return nullptr;    // 判断空指针。

    string str;
    timetostr(ttime,str,fmt);           // 直接调用string& timetostr(const time_t ttime,string &strtime,const string &fmt="");
    str.copy(strtime,str.length());
    strtime[str.length()]=0;           // string的copy函数不会给C风格字符串的结尾加0。

    return strtime;
}

string timetostr1(const time_t ttime,const string &fmt)
{
    string str;
    timetostr(ttime,str,fmt);           // 直接调用string& timetostr(const time_t ttime,string &strtime,const string &fmt="");
    return str;
}

string& ltime(string &strtime,const string &fmt,const int timetvl)
{
    time_t  timer;
    time(&timer );                          // 获取系统当前时间。

    timer=timer+timetvl;              // 加上时间的偏移量。

    timetostr(timer,strtime,fmt);   // 把整数表示的时间转换为字符串表示的时间。

    return strtime;
}

char* ltime(char *strtime,const string &fmt,const int timetvl)
{
    if (strtime==nullptr) return nullptr;    // 判断空指针。

    time_t  timer;
    time(&timer );                          // 获取系统当前时间。

    timer=timer+timetvl;              // 加上时间的偏移量。

    timetostr(timer,strtime,fmt);   // 把整数表示的时间转换为字符串表示的时间。

    return strtime;
}

string ltime1(const string &fmt,const int timetvl)
{
    string strtime;

    ltime(strtime,fmt,timetvl);   // 直接调用string& ltime(string &strtime,const string &fmt="",const int timetvl=0);

    return strtime;
}

bool clogfile::open(const string &filename,const ios::openmode mode,const bool bbackup,const bool benbuffer)
{
    // 如果日志文件是打开的状态，先关闭它。
    if (fout.is_open()) fout.close();

    m_filename=filename;        // 日志文件名。
    m_mode=mode;                 // 打开模式。
    m_backup=bbackup;          // 是否自动备份。
    m_enbuffer=benbuffer;      // 是否启用文件缓冲区。

    newdir(m_filename,true);                              // 如果日志文件的目录不存在，创建它。

    fout.open(m_filename,m_mode);                  // 打开日志文件。

    if (m_enbuffer==false) fout << unitbuf;       // 是否启用文件缓冲区。

    return fout.is_open();
}

bool clogfile::backup()
{
    // 不备份
    if (m_backup == false) return true;

    if (fout.is_open() == false) return false;

    // 如果当前日志文件的大小超过m_maxsize，备份日志。
    if (fout.tellp() > m_maxsize*1024*1024)
    {
        m_splock.lock();       // 加锁。

        fout.close();              // 关闭当前日志文件。

        // 拼接备份日志文件名。
        string bak_filename=m_filename+"."+ltime1("yyyymmddhh24miss");

        rename(m_filename.c_str(),bak_filename.c_str());   // 把当前日志文件改名为备份日志文件。

        fout.open(m_filename,m_mode);              // 重新打开当前日志文件。

        if (m_enbuffer==false) fout << unitbuf;   // 判断是否启动文件缓冲区。

        m_splock.unlock();   // 解锁。

        return fout.is_open();
    }

    return true;
}

bool cifile::open(const string &filename,const ios::openmode mode)
{
    // 如果文件是打开的状态，先关闭它。
    if (fin.is_open()) fin.close();

    m_filename=filename;

    fin.open(m_filename,mode);

    return fin.is_open();
}

int cifile::read(void *buf,const int bufsize)
{
    // fin.read((char *)buf,bufsize);
    fin.read(static_cast<char *>(buf),bufsize);

    return fin.gcount();          // 返回读取的字节数。
}

bool cifile::closeandremove()
{
    if (fin.is_open()==false) return false;

    fin.close(); 

    if (remove(m_filename.c_str())!=0) return false;

    return true;
}

void cifile::close() 
{ 
    if (fin.is_open()==false) return;

    fin.close(); 
}

bool cifile::readline(string &buf,const string& endbz)
{
    buf.clear();            // 清空buf。

    string strline;        // 存放从文件中读取的一行。

    while (true)
    {
        getline(fin,strline);    // 从文件中读取一行。
      
        if (fin.eof()) break;    // 如果文件已读完。

        buf=buf+strline;      // 把读取的内容拼接到buf中。

        if (endbz=="")
            return true;          // 如果行没有结尾标志。
        else 
        {
            // 如果行有结尾标志，判断本次是否读到了结尾标志，如果没有，继续读，如果有，返回。
            if (buf.find(endbz,buf.length()-endbz.length()) != string::npos) return true;
        }

        buf=buf+"\n";        // getline从文件中读取一行的时候，会删除\n，所以，这里要补上\n，因为这个\n不应该被删除。
    }

    return false;
}

bool cofile::open(const string &filename,const bool btmp,const ios::openmode mode,const bool benbuffer)
{
    // 如果文件是打开的状态，先关闭它。
    if (fout.is_open()) fout.close();

    m_filename=filename;

    newdir(m_filename,true);     // 如果文件的目录不存在，创建目录。

    if (btmp==true) 
    {   // 采用临时文件的方案。
        m_filenametmp=m_filename+".tmp";
        fout.open(m_filenametmp,mode);
    }
    else
    {   // 不采用临时文件的方案。
        m_filenametmp.clear();
        fout.open(m_filename,mode);
    }

    // 不启用文件缓冲区。
    if (benbuffer==false) fout << unitbuf;

    return fout.is_open();
}

bool cofile::write(void *buf,int bufsize)
{
    if (fout.is_open()==false) return false;

    // fout.write((char *)buf,bufsize);
    fout.write(static_cast<char *>(buf),bufsize);

    return fout.good();
}

// 关闭文件，并且把临时文件名改为正式文件名。
bool cofile::closeandrename()
{
    if (fout.is_open()==false) return false;

    fout.close();

    //  如果采用了临时文件的方案。
    if (m_filenametmp.empty()==false) 
        if (rename(m_filenametmp.c_str(),m_filename.c_str())!=0) return false;

    return true;
}

// 关闭文件，删除临时文件。
void cofile::close() 
{ 
    if (fout.is_open()==false) return;

    fout.close(); 

    //  如果采用了临时文件的方案。
    if (m_filenametmp.empty()==false) 
        remove(m_filenametmp.c_str());
}

bool newdir(const string &pathorfilename,bool bisfilename)
{
    // /tmp/aaa/bbb/ccc/ddd    /tmp    /tmp/aaa    /tmp/aaa/bbb    /tmp/aaa/bbb/ccc 
     
    // 检查目录是否存在，如果不存在，逐级创建子目录
    int pos=1;          // 不要从0开始，0是根目录/。

    while (true)
    {
        int pos1=pathorfilename.find('/',pos);
        if (pos1==string::npos) break;

        string strpathname=pathorfilename.substr(0,pos1);      // 截取目录。

        pos=pos1+1;       // 位置后移。
        if (access(strpathname.c_str(),F_OK) != 0)  // 如果目录不存在，创建它。
        {
            // 0755是八进制，不要写成755。
            if (mkdir(strpathname.c_str(),0755) != 0) return false;  // 如果目录不存在，创建它。
        }
    }

    // 如果pathorfilename不是文件，是目录，还需要创建最后一级子目录。
    if (bisfilename==false)
    {
        if (access(pathorfilename.c_str(),F_OK) != 0)
        {
            if (mkdir(pathorfilename.c_str(),0755) != 0) return false;
        }
    }

    return true;
}

int filesize(const string &filename)
{
    struct stat st_filestat;      // 存放文件信息的结构体。

    // 获取文件信息，存放在结构体中。
    if (stat(filename.c_str(),&st_filestat) < 0) return -1;

    return st_filestat.st_size;   // 返回结构体的文件大小成员。
}

bool setmtime(const string &filename,const string &mtime)
{
    struct utimbuf stutimbuf;

    stutimbuf.actime=stutimbuf.modtime=strtotime(mtime);

    if (utime(filename.c_str(),&stutimbuf)!=0) return false;

    return true;
}

time_t strtotime(const string &strtime)
{
    string strtmp,yyyy,mm,dd,hh,mi,ss;

    picknumber(strtime,strtmp,false,false);    // 把字符串中的数字全部提取出来。
    // 2021-12-05 08:30:45
    // 2021/12/05 08:30:45
    // 20211205083045

    if (strtmp.length() != 14) return -1;           // 如果时间格式不是yyyymmddhh24miss，说明时间格式不正确。

    yyyy=strtmp.substr(0,4);
    mm=strtmp.substr(4,2);
    dd=strtmp.substr(6,2);
    hh=strtmp.substr(8,2);
    mi=strtmp.substr(10,2);
    ss=strtmp.substr(12,2);

    struct tm sttm;

    try
    {
        sttm.tm_year = stoi(yyyy) - 1900;
        sttm.tm_mon = stoi(mm) - 1;
        sttm.tm_mday = stoi(dd);
        sttm.tm_hour = stoi(hh);
        sttm.tm_min = stoi(mi);
        sttm.tm_sec = stoi(ss);
        sttm.tm_isdst = 0;
    }
    catch(const std::exception& e)
    {
        return -1;
    }

    return mktime(&sttm);
}

bool addtime(const string &in_stime,string &out_stime,const int timetvl,const string &fmt)
{
    time_t  timer;

    // 把字符串表示的时间转换为整数表示的时间，方便运算。
    if ( (timer=strtotime(in_stime))==-1) { out_stime=""; return false; }

    timer=timer+timetvl;  // 时间运算。

    // 把整数表示的时间转换为字符串表示的时间。
    timetostr(timer,out_stime,fmt);

    return true;
}

bool addtime(const string &in_stime,char *out_stime,const int timetvl,const string &fmt)
{
    if (out_stime==nullptr) return false;    // 判断空指针。

    time_t  timer;

    // 把字符串表示的时间转换为整数表示的时间，方便运算。
    if ( (timer=strtotime(in_stime))==-1) { strcpy(out_stime,""); return false; }

    timer=timer+timetvl;  // 时间运算。

    // 把整数表示的时间转换为字符串表示的时间。
    timetostr(timer,out_stime,fmt);

    return true;
}

bool filemtime(const string &filename,string &mtime,const string &fmt)
{
    struct stat st_filestat;      // 存放文件信息的结构体。

    // 获取文件信息，存放在结构体中。
    if (stat(filename.c_str(),&st_filestat) < 0) return false;

    // 把整数表示的时间转换成字符串表示的时间。
    timetostr(st_filestat.st_mtime,mtime,fmt);

    return true;
}

bool filemtime(const string &filename,char *mtime,const string &fmt)
{
    struct stat st_filestat;      // 存放文件信息的结构体。

    // 获取文件信息，存放在结构体中。
    if (stat(filename.c_str(),&st_filestat) < 0) return false;

    // 把整数表示的时间转换成字符串表示的时间。
    timetostr(st_filestat.st_mtime,mtime,fmt);

    return true;
}

void cdir::setfmt(const string &fmt)
{
    m_fmt=fmt;
}

bool cdir::opendir(const string &dirname,const string &rules,const int maxfiles,const bool bandchild,bool bsort)
{
    m_filelist.clear();    // 清空文件列表容器。
    m_pos=0;              // 从文件列表中已读取文件的位置归0。

    // 如果目录不存在，创建它。
    if (newdir(dirname,false) == false) return false;

    // 打开目录，获取目录中的文件列表，存放在m_filelist容器中。
    bool ret=_opendir(dirname,rules,maxfiles,bandchild);

    if (bsort==true)    // 对文件列表排序。
    {
      sort(m_filelist.begin(), m_filelist.end());
    }

    return ret;
}

// 这是一个递归函数，在opendir()中调用，cdir类的外部不需要调用它。
bool cdir::_opendir(const string &dirname,const string &rules,const int maxfiles,const bool bandchild)
{
    DIR *dir;   // 目录指针。

    // 打开目录。
    if ( (dir=::opendir(dirname.c_str())) == nullptr ) return false; // opendir与库函数重名，需要加::

    string strffilename;            // 全路径的文件名。
    struct dirent *stdir;            // 存放从目录中读取的内容。

    // 用循环读取目录的内容，将得到目录中的文件名和子目录。
    while ((stdir=::readdir(dir)) != 0) // readdir与库函数重名，需要加::
    {
        // 判断容器中的文件数量是否超出maxfiles参数。
        if ( m_filelist.size()>=maxfiles ) break;

        // 文件名以"."打头的文件不处理。.是当前目录，..是上一级目录，其它以.打头的都是特殊目录和文件。
        if (stdir->d_name[0]=='.') continue;
        
        // 拼接全路径的文件名。
        strffilename=dirname+'/'+stdir->d_name;  

        // 如果是目录，处理各级子目录。
        if (stdir->d_type==4)
        {
            if (bandchild == true)      // 打开各级子目录。
            {
                if (_opendir(strffilename,rules,maxfiles,bandchild) == false)   // 递归调用_opendir函数。
                {
                    closedir(dir); return false;
                }
            }
        }
        
        // 如果是普通文件，放入容器中。
        if (stdir->d_type==8)
        {
            // 把能匹配上的文件放入m_filelist容器中。
            if (matchstr(stdir->d_name,rules) == false) continue;

            m_filelist.push_back(std::move(strffilename));
        }
    }

    closedir(dir);   // 关闭目录。

    return true;
}

bool cdir::readdir()
{
    // 如果已读完，清空容器
    if (m_pos >= m_filelist.size()) 
    {
      m_pos=0; m_filelist.clear(); return false;
    }

    // 文件全名，包括路径
    m_ffilename=m_filelist[m_pos];

    // 从绝对路径的文件名中解析出目录名和文件名。
    int pp=m_ffilename.find_last_of("/");
    m_dirname=m_ffilename.substr(0,pp);
    m_filename=m_ffilename.substr(pp+1);

    // 获取文件的信息。
    struct stat st_filestat;
    stat(m_ffilename.c_str(),&st_filestat);
    m_filesize=st_filestat.st_size;                                     // 文件大小。
    m_mtime=timetostr1(st_filestat.st_mtime,m_fmt);   // 文件最后一次被修改的时间。
    m_ctime=timetostr1(st_filestat.st_ctime,m_fmt);      // 文件生成的时间。
    m_atime=timetostr1(st_filestat.st_atime,m_fmt);      // 文件最后一次被访问的时间。

    m_pos++;       // 已读取文件的位置后移。

    return true;
}

cdir::~cdir()
{
    m_filelist.clear();
}

bool renamefile(const string &srcfilename,const string &dstfilename)
{
    // 如果原文件不存在，直接返回失败。
    if (access(srcfilename.c_str(),R_OK) != 0) return false;

    // 创建目标文件的目录。
    if (newdir(dstfilename,true) == false) return false;

    // 调用操作系统的库函数rename重命名文件。 mv
    if (rename(srcfilename.c_str(),dstfilename.c_str()) == 0) return true;

    return false;
}

// 忽略关闭全部的信号、关闭全部的IO，缺省只忽略信号，不关IO。 
// 不希望后台服务程序被信号打扰，需要什么信号可以在程序中设置。
// 实际上关闭的IO是0、1、2。
void closeioandsignal(bool bcloseio)
{
    int ii=0;

    for (ii=0;ii<64;ii++)
    {
        if (bcloseio==true) close(ii);

        signal(ii,SIG_IGN); 
    }
}

bool ctcpclient::connect(const string &ip,const int port)
{
    // 如果已连接到服务端，则断开，这种处理方法没有特别的原因，不要纠结。
    if (m_connfd!=-1) { ::close(m_connfd); m_connfd=-1; }
 
    // 忽略SIGPIPE信号，防止程序异常退出。
    // 如果send到一个disconnected socket上，内核就会发出SIGPIPE信号。这个信号
    // 的缺省处理方法是终止进程，大多数时候这都不是我们期望的。我们重新定义这
    // 个信号的处理方法，大多数情况是直接屏蔽它。
    signal(SIGPIPE,SIG_IGN);   

    m_ip=ip;
    m_port=port;

    struct hostent* h;
    struct sockaddr_in servaddr;

    if ( (m_connfd = socket(AF_INET,SOCK_STREAM,0) ) < 0) return false;

    if ( !(h = gethostbyname(m_ip.c_str())) )
    {
        ::close(m_connfd);  m_connfd=-1; return false;
    }

    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(m_port);  // 指定服务端的通讯端口
    memcpy(&servaddr.sin_addr,h->h_addr,h->h_length);

    if (::connect(m_connfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) != 0)
    {
        ::close(m_connfd);  m_connfd=-1; return false;
    }

    return true;
}

void ctcpclient::close()
{
    if (m_connfd >= 0) ::close(m_connfd); 

    m_connfd=-1;
    m_port=0;
}

ctcpclient::~ctcpclient()
{
    close();
}

bool ctcpserver::initserver(const unsigned int port,const int backlog)
{
    // 如果服务端的socket>0，关掉它，这种处理方法没有特别的原因，不要纠结。
    if (m_listenfd > 0) { ::close(m_listenfd); m_listenfd=-1; }

    if ( (m_listenfd = socket(AF_INET,SOCK_STREAM,0))<=0) return false;

    // 忽略SIGPIPE信号，防止程序异常退出。
    // 如果往已关闭的socket继续写数据，会产生SIGPIPE信号，它的缺省行为是终止程序，所以要忽略它。
    signal(SIGPIPE,SIG_IGN);   

    // 打开SO_REUSEADDR选项，当服务端连接处于TIME_WAIT状态时可以再次启动服务器，
    // 否则bind()可能会不成功，报：Address already in use。
    int opt = 1; 
    setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));    

    memset(&m_servaddr,0,sizeof(m_servaddr));
    m_servaddr.sin_family = AF_INET;
    m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);   // 任意ip地址。
    m_servaddr.sin_port = htons(port);
    if (bind(m_listenfd,(struct sockaddr *)&m_servaddr,sizeof(m_servaddr)) != 0 )
    {
        closelisten(); return false;
    }

    if (listen(m_listenfd,backlog) != 0 )
    {
        closelisten(); return false;
    }

    return true;
}

bool ctcpserver::accept()
{
    if (m_listenfd==-1) return false;

    int m_socklen = sizeof(struct sockaddr_in);
    if ((m_connfd=::accept(m_listenfd,(struct sockaddr *)&m_clientaddr,(socklen_t*)&m_socklen)) < 0)
        return false;

    return true;
}

char *ctcpserver::getip()
{
    return(inet_ntoa(m_clientaddr.sin_addr));
}

bool ctcpserver::read(void *buffer,const int ibuflen,const int itimeout)   // 接收二进制数据。
{
    if (m_connfd==-1) return false;

    return(tcpread(m_connfd,buffer,ibuflen,itimeout));
}

bool ctcpserver::read(string &buffer,const int itimeout)  // 接收文本数据。
{
    if (m_connfd==-1) return false;

    return(tcpread(m_connfd,buffer,itimeout));
}

bool ctcpclient::read(void *buffer,const int ibuflen,const int itimeout)   // 接收二进制数据。
{
    if (m_connfd==-1) return false;

    return(tcpread(m_connfd,buffer,ibuflen,itimeout));
}

bool ctcpclient::read(string &buffer,const int itimeout)  // 接收文本数据。
{
    if (m_connfd==-1) return false;

    return(tcpread(m_connfd,buffer,itimeout));
}

bool ctcpserver::write(const void *buffer,const int ibuflen)  // 发送二进制数据。
{
    if (m_connfd==-1) return false;

    return(tcpwrite(m_connfd,(char*)buffer,ibuflen));
}

bool ctcpserver::write(const string &buffer)
{
    if (m_connfd==-1) return false;

    return(tcpwrite(m_connfd,buffer));
}

bool ctcpclient::write(const void *buffer,const int ibuflen)
{
    if (m_connfd==-1) return false;

    return(tcpwrite(m_connfd,(char*)buffer,ibuflen));
}

bool ctcpclient::write(const string &buffer)
{
    if (m_connfd==-1) return false;

    return(tcpwrite(m_connfd,buffer));
}

void ctcpserver::closelisten()
{
    if (m_listenfd >= 0)
    {
        ::close(m_listenfd); m_listenfd=-1;
    }
}

void ctcpserver::closeclient()
{
    if (m_connfd >= 0)
    {
        ::close(m_connfd); m_connfd=-1; 
    }
}

ctcpserver::~ctcpserver()
{
    closelisten(); closeclient();
}

bool tcpread(const int sockfd,void *buffer,const int ibuflen,const int itimeout)    // 接收二进制数据。
{
    if (sockfd==-1) return false;

    // 如果itimeout>0，表示需要等待itimeout秒，如果itimeout秒后还没有数据到达，返回false。
    if (itimeout>0)
    {
        struct pollfd fds;
        fds.fd=sockfd;
        fds.events=POLLIN;
        if ( poll(&fds,1,itimeout*1000) <= 0 ) return false;
    }

    // 如果itimeout==-1，表示不等待，立即判断socket的缓冲区中是否有数据，如果没有，返回false。
    if (itimeout==-1)
    {
        struct pollfd fds;
        fds.fd=sockfd;
        fds.events=POLLIN;
        if ( poll(&fds,1,0) <= 0 ) return false;
    }

    // 读取报文内容。
    if (readn(sockfd,(char*)buffer,ibuflen) == false) return false;

    return true;
}

bool tcpread(const int sockfd,string &buffer,const int itimeout)    // 接收文本数据。
{
    if (sockfd==-1) return false;

    // 如果itimeout>0，表示等待itimeout秒，如果itimeout秒后接收缓冲区中还没有数据，返回false。
    if (itimeout>0)
    {
        struct pollfd fds;
        fds.fd=sockfd;
        fds.events=POLLIN;
        if ( poll(&fds,1,itimeout*1000) <= 0 ) return false;
    }

    // 如果itimeout==-1，表示不等待，立即判断socket的接收缓冲区中是否有数据，如果没有，返回false。
    if (itimeout==-1)
    {
        struct pollfd fds;
        fds.fd=sockfd;
        fds.events=POLLIN;
        if ( poll(&fds,1,0) <= 0 ) return false;
    }

    int buflen=0;

    // 先读取报文长度，4个字节。
    if (readn(sockfd,(char*)&buflen,4) == false) return false;

    buffer.resize(buflen);   // 设置buffer的大小。

    // 再读取报文内容。
    if (readn(sockfd,&buffer[0],buflen) == false) return false;

    return true;
}

bool tcpwrite(const int sockfd,const void *buffer,const int ibuflen)        // 发送二进制数据。
{
    if (sockfd==-1) return false;

    if (writen(sockfd,(char*)buffer,ibuflen) == false) return false;

    return true;
}

bool tcpwrite(const int sockfd,const string &buffer)      // 发送文本数据。
{
    if (sockfd==-1) return false;

    int buflen=buffer.size();

    // 先发送报头。
    if (writen(sockfd,(char*)&buflen,4) == false) return false;

    // 再发送报文体。
    if (writen(sockfd,buffer.c_str(),buflen) == false) return false;

    return true;
}

// 从已经准备好的socket中读取数据。
// sockfd：已经准备好的socket连接。
// buffer：接收数据缓冲区的地址。
// n：本次接收数据的字节数。
// 返回值：成功接收到n字节的数据后返回true，socket连接不可用返回false。
bool readn(const int sockfd,char *buffer,const size_t n)
{
    int nleft=n;    // 剩余需要读取的字节数。
    int idx=0;       // 已成功读取的字节数。
    int nread;       // 每次调用recv()函数读到的字节数。

    while(nleft > 0)
    {
        if ( (nread=recv(sockfd,buffer+idx,nleft,0)) <= 0) return false;

        idx=idx+nread;
        nleft=nleft-nread;
    }

    return true;
}

// 向已经准备好的socket中写入数据。
// sockfd：已经准备好的socket连接。
// buffer：待发送数据缓冲区的地址。
// n：待发送数据的字节数。
// 返回值：成功发送完n字节的数据后返回true，socket连接不可用返回false。
bool writen(const int sockfd,const char *buffer,const size_t n)
{
    int nleft=n;       // 剩余需要写入的字节数。
    int idx=0;          // 已成功写入的字节数。
    int nwritten;      // 每次调用send()函数写入的字节数。
  
    while(nleft > 0 )
    {    
      if ( (nwritten=send(sockfd,buffer+idx,nleft,0)) <= 0) return false;      
    
      nleft=nleft-nwritten;
      idx=idx+nwritten;
    }

    return true;
}

bool copyfile(const string &srcfilename,const string &dstfilename)
{
    // 创建目标文件的目录。
    if (newdir(dstfilename,true) == false) return false;

    cifile ifile;
    cofile ofile;
    int ifilesize=filesize(srcfilename);

    int  total_bytes=0;
    int  onread=0;
    char buffer[5000];

    if (ifile.open(srcfilename,ios::in|ios::binary)==false) return false;

    if (ofile.open(dstfilename,ios::out|ios::binary)==false) return false;

    while (true)
    {
        if ((ifilesize-total_bytes) > 5000) onread=5000;
        else onread=ifilesize-total_bytes;

        memset(buffer,0,sizeof(buffer));
        ifile.read(buffer,onread);
        ofile.write(buffer,onread);

        total_bytes = total_bytes + onread;

        if (total_bytes == ifilesize) break;
    }

    ifile.close();
    ofile.closeandrename();

    // 更改文件的修改时间属性
    string strmtime;
    filemtime(srcfilename,strmtime);
    setmtime(dstfilename,strmtime);

    return true;
}

ctimer::ctimer()
{
    start();   // 计时开始。
}

// 计时开始。
void ctimer::start()
{
    memset(&m_start,0,sizeof(struct timeval));
    memset(&m_end,0,sizeof(struct timeval));

    gettimeofday(&m_start, 0);    // 获取当前时间，精确到微秒。
}

// 计算已逝去的时间，单位：秒，小数点后面是微秒
// 每调用一次本方法之后，自动调用Start方法重新开始计时。
double ctimer::elapsed()
{
    gettimeofday(&m_end,0);     // 获取当前时间作为计时结束的时间，精确到微秒。

    string str;
    str=sformat("%ld.%06ld",m_start.tv_sec,m_start.tv_usec);
    double dstart=stod(str);      // 把计时开始的时间点转换为double。

    str=sformat("%ld.%06ld",m_end.tv_sec,m_end.tv_usec);
    double dend=stod(str);       // 把计时结束的时间点转换为double。

    start();                                  // 重新开始计时。

    return dend-dstart;
}

 cpactive::cpactive()
 {
     m_shmid=0;
     m_pos=-1;
     m_shm=0;
 }

 // 把当前进程的信息加入共享内存进程组中。
 bool cpactive::addpinfo(const int timeout,const string &pname,clogfile *logfile)
 {
    if (m_pos!=-1) return true;

    // 创建/获取共享内存，键值为SHMKEYP，大小为MAXNUMP个st_procinfo结构体的大小。
    if ( (m_shmid = shmget((key_t)SHMKEYP, MAXNUMP*sizeof(struct st_procinfo), 0666|IPC_CREAT)) == -1)
    { 
        if (logfile!=nullptr) logfile->write("创建/获取共享内存(%x)失败。\n",SHMKEYP); 
        else printf("创建/获取共享内存(%x)失败。\n",SHMKEYP);

        return false; 
    }

    // 将共享内存连接到当前进程的地址空间。
    m_shm=(struct st_procinfo *)shmat(m_shmid, 0, 0);
  
    /*
    struct st_procinfo stprocinfo;    // 当前进程心跳信息的结构体。
    memset(&stprocinfo,0,sizeof(stprocinfo));
    stprocinfo.pid=getpid();            // 当前进程号。
    stprocinfo.timeout=timeout;         // 超时时间。
    stprocinfo.atime=time(0);           // 当前时间。
    strncpy(stprocinfo.pname,pname.c_str(),50); // 进程名。
    */
    st_procinfo stprocinfo(getpid(),pname.c_str(),timeout,time(0));    // 当前进程心跳信息的结构体。

    // 进程id是循环使用的，如果曾经有一个进程异常退出，没有清理自己的心跳信息，
    // 它的进程信息将残留在共享内存中，不巧的是，如果当前进程重用了它的id，
    // 守护进程检查到残留进程的信息时，会向进程id发送退出信号，将误杀当前进程。
    // 所以，如果共享内存中已存在当前进程编号，一定是其它进程残留的信息，当前进程应该重用这个位置。
    for (int ii=0;ii<MAXNUMP;ii++)
    {
        if ( (m_shm+ii)->pid==stprocinfo.pid ) { m_pos=ii; break; }
    }

    csemp semp;                       // 用于给共享内存加锁的信号量id。

    if (semp.init(SEMKEYP) == false)  // 初始化信号量。
    {
        if (logfile!=nullptr) logfile->write("创建/获取信号量(%x)失败。\n",SEMKEYP); 
        else printf("创建/获取信号量(%x)失败。\n",SEMKEYP);

        return false;
    }

    semp.wait();  // 给共享内存上锁。

    // 如果m_pos==-1，表示共享内存的进程组中不存在当前进程编号，那就找一个空位置。
    if (m_pos==-1)
    {
        for (int ii=0;ii<MAXNUMP;ii++)
            if ( (m_shm+ii)->pid==0 ) { m_pos=ii; break; }
    }

    // 如果m_pos==-1，表示没找到空位置，说明共享内存的空间已用完。
    if (m_pos==-1) 
    { 
        if (logfile!=0) logfile->write("共享内存空间已用完。\n");
        else printf("共享内存空间已用完。\n");

        semp.post();  // 解锁。

        return false; 
    }

    // 把当前进程的心跳信息存入共享内存的进程组中。
    memcpy(m_shm+m_pos,&stprocinfo,sizeof(struct st_procinfo)); 

    semp.post();   // 解锁。

    return true;
 }

 // 更新共享内存进程组中当前进程的心跳时间。
 bool cpactive::uptatime()
 {
    if (m_pos==-1) return false;

    (m_shm+m_pos)->atime=time(0);

    return true;
 }

 cpactive::~cpactive()
 {
    // 把当前进程从共享内存的进程组中移去。
    if (m_pos!=-1) memset(m_shm+m_pos,0,sizeof(struct st_procinfo));

    // 把共享内存从当前进程中分离。
    if (m_shm!=0) shmdt(m_shm);
 }

// 如果信号量已存在，获取信号量；如果信号量不存在，则创建它并初始化为value。
// 如果用于互斥锁，value填1，sem_flg填SEM_UNDO。
// 如果用于生产消费者模型，value填0，sem_flg填0。
bool csemp::init(key_t key,unsigned short value,short sem_flg)
{
    if (m_semid!=-1) return false; // 如果已经初始化了，不必再次初始化。

    m_sem_flg=sem_flg;

    // 信号量的初始化不能直接用semget(key,1,0666|IPC_CREAT)
    // 因为信号量创建后，初始值是0，如果用于互斥锁，需要把它的初始值设置为1，
    // 而获取信号量则不需要设置初始值，所以，创建信号量和获取信号量的流程不同。

    // 信号量的初始化分三个步骤：
    // 1）获取信号量，如果成功，函数返回。
    // 2）如果失败，则创建信号量。
    // 3) 设置信号量的初始值。

    // 获取信号量。
    if ( (m_semid=semget(key,1,0666)) == -1)
    {
        // 如果信号量不存在，创建它。
        if (errno==ENOENT)
        {
            // 用IPC_EXCL标志确保只有一个进程创建并初始化信号量，其它进程只能获取。
            if ( (m_semid=semget(key,1,0666|IPC_CREAT|IPC_EXCL)) == -1)
            {
                if (errno==EEXIST) // 如果错误代码是信号量已存在，则再次获取信号量。
                {
                    if ( (m_semid=semget(key,1,0666)) == -1)
                    { 
                        perror("init 1 semget()"); return false; 
                    }
                    return true;
                }
                else  // 如果是其它错误，返回失败。
                {
                    perror("init 2 semget()"); return false;
                }
            }

            // 信号量创建成功后，还需要把它初始化成value。
            union semun sem_union;
            sem_union.val = value;   // 设置信号量的初始值。
            if (semctl(m_semid,0,SETVAL,sem_union) <  0) 
            { 
                perror("init semctl()"); return false; 
            }
        }
        else
        { perror("init 3 semget()"); return false; }
    }

    return true;
}

// 信号量的P操作（把信号量的值减value），如果信号量的值是0，将阻塞等待，直到信号量的值大于0。
bool csemp::wait(short value)
{
    if (m_semid==-1) return false;

    struct sembuf sem_b;
    sem_b.sem_num = 0;      // 信号量编号，0代表第一个信号量。
    sem_b.sem_op = value;   // P操作的value必须小于0。
    sem_b.sem_flg = m_sem_flg;
    if (semop(m_semid,&sem_b,1) == -1) { perror("p semop()"); return false; }

    return true;
}

// 信号量的V操作（把信号量的值减value）。
bool csemp::post(short value)
{
    if (m_semid==-1) return false;

    struct sembuf sem_b;
    sem_b.sem_num = 0;     // 信号量编号，0代表第一个信号量。
    sem_b.sem_op = value;  // V操作的value必须大于0。
    sem_b.sem_flg = m_sem_flg;
    if (semop(m_semid,&sem_b,1) == -1) { perror("V semop()"); return false; }

    return true;
}

// 获取信号量的值，成功返回信号量的值，失败返回-1。
int csemp::getvalue()
{
    return semctl(m_semid,0,GETVAL);
}

// 销毁信号量。
bool csemp::destroy()
{
    if (m_semid==-1) return false;

    if (semctl(m_semid,0,IPC_RMID) == -1) { perror("destroy semctl()"); return false; }

    return true;
}

csemp::~csemp()
{
}


} // namespace
