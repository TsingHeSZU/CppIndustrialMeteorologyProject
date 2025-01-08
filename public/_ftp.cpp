/****************************************************************************************/
/*   程序名：_ftp.cpp，此程序是开发框架的ftp客户端工具的类的定义文件。                  */
/*   作者：吴从周 
/****************************************************************************************/

#include "_ftp.h"

namespace idc
{

cftpclient::cftpclient()
{
    m_ftpconn=0;

    initdata();

    FtpInit();

    m_connectfailed=false;
    m_loginfailed=false;
    m_optionfailed=false;
}

cftpclient::~cftpclient()
{
    logout();
}

void cftpclient::initdata()
{
    m_size=0;

    m_mtime.clear();
}

bool cftpclient::login(const string &host,const string &username,const string &password,const int imode)
{
    if (m_ftpconn != 0) { FtpQuit(m_ftpconn); m_ftpconn=0; }

    m_connectfailed=m_loginfailed=m_optionfailed=false;

    if (FtpConnect(host.c_str(),&m_ftpconn) == false)  { m_connectfailed=true; return false; }

    if (FtpLogin(username.c_str(),password.c_str(),m_ftpconn) == false)  { m_loginfailed=true; return false; }

    if (FtpOptions(FTPLIB_CONNMODE,(long)imode,m_ftpconn) == false) { m_optionfailed=true; return false; }

    return true;
}

bool cftpclient::logout()
{
    if (m_ftpconn == 0) return false;

    FtpQuit(m_ftpconn);

    m_ftpconn=0;

    return true;
}

bool cftpclient::get(const string &remotefilename,const string &localfilename,const bool bcheckmtime)
{
    if (m_ftpconn == 0) return false;

    // 创建本地文件目录。
    newdir(localfilename);

    // 生成本地文件的临时文件名。
    string strlocalfilenametmp=localfilename+".tmp";

    // 获取远程服务器的文件的时间。
    if (mtime(remotefilename) == false) return false;

    // 取文件。
    if (FtpGet(strlocalfilenametmp.c_str(),remotefilename.c_str(),FTPLIB_IMAGE,m_ftpconn) == false) return false;
  
    // 判断文件下载前和下载后的时间，如果时间不同，表示在文件传输的过程中已发生了变化，返回失败。
    if (bcheckmtime==true)
    {
        string strmtime=m_mtime;

        if (mtime(remotefilename) == false) return false;

        if (m_mtime!=strmtime) return false;
    }

    // 重置文件时间。
    setmtime(strlocalfilenametmp,m_mtime);

    // 改为正式的文件。
    if (rename(strlocalfilenametmp.c_str(),localfilename.c_str()) != 0) return false; 

    // 获取文件的大小。
    m_size=filesize(localfilename);

    return true;
}

bool cftpclient::mtime(const string &remotefilename)
{
    if (m_ftpconn == 0) return false;
  
    m_mtime.clear();
  
    string strmtime;
    strmtime.resize(14);

    if (FtpModDate(remotefilename.c_str(),&strmtime[0],14,m_ftpconn) == false) return false;

    // 把UTC时间转换为本地时间。
    addtime(strmtime,m_mtime,0+8*60*60,"yyyymmddhh24miss");

    return true;
}

bool cftpclient::size(const string &remotefilename)
{
    if (m_ftpconn == 0) return false;

    m_size=0;
  
    if (FtpSize(remotefilename.c_str(),&m_size,FTPLIB_IMAGE,m_ftpconn) == false) return false;

    return true;
}

bool cftpclient::chdir(const string &remotedir)
{
    if (m_ftpconn == 0) return false;
  
    if (FtpChdir(remotedir.c_str(),m_ftpconn) == false) return false;

    return true;
}

bool cftpclient::mkdir(const string &remotedir)
{
    if (m_ftpconn == 0) return false;
  
    if (FtpMkdir(remotedir.c_str(),m_ftpconn) == false) return false;

    return true;
}

bool cftpclient::rmdir(const string &remotedir)
{
    if (m_ftpconn == 0) return false;
  
    if (FtpRmdir(remotedir.c_str(),m_ftpconn) == false) return false;

    return true;
}

bool cftpclient::nlist(const string &remotedir,const string &listfilename)
{
    if (m_ftpconn == 0) return false;

    newdir(listfilename.c_str()); // 创建本地list文件目录
  
    if (FtpNlst(listfilename.c_str(),remotedir.c_str(),m_ftpconn) == false) return false;

    return true;
}

bool cftpclient::put(const string &localfilename,const string &remotefilename,const bool bchecksize)
{
    if (m_ftpconn == 0) return false;

    // 生成服务器文件的临时文件名。
    string strremotefilenametmp=remotefilename+".tmp";

    string filetime1,filetime2;
    filemtime(localfilename,filetime1);   // 获取上传文件之前的时间。

    // 发送文件。
    if (FtpPut(localfilename.c_str(),strremotefilenametmp.c_str(),FTPLIB_IMAGE,m_ftpconn) == false) return false;

    filemtime(localfilename,filetime2);   // 获取上传文件之后的时间。

    // 如果文件上传前后的时间不一致，说明本地有修改文件，放弃本次上传。
    if (filetime1!=filetime2) { ftpdelete(strremotefilenametmp); return false; }
    
    // 重命名文件。
    if (FtpRename(strremotefilenametmp.c_str(),remotefilename.c_str(),m_ftpconn) == false) return false;

    // 判断已上传的文件的大小与本地文件是否相同，确保上传成功。
    // 一般来说，不会出现文件大小不一致的情况，如果有，应该是服务器方的原因，不太好处理。
    if (bchecksize==true)
    {
        if (size(remotefilename) == false) return false;

        if (m_size != filesize(localfilename)) { ftpdelete(remotefilename); return false; }
    }

    return true;
}

bool cftpclient::ftpdelete(const string &remotefilename)
{
    if (m_ftpconn == 0) return false;

    if (FtpDelete(remotefilename.c_str(),m_ftpconn) == false) return false;
  
    return true;
}

bool cftpclient::ftprename(const string &srcremotefilename,const string &dstremotefilename)
{
    if (m_ftpconn == 0) return false;

    if (FtpRename(srcremotefilename.c_str(),dstremotefilename.c_str(),m_ftpconn) == false) return false;
  
    return true;
}

bool cftpclient::site(const string &command)
{
    if (m_ftpconn == 0) return false;
  
    if (FtpSite(command.c_str(),m_ftpconn) == false) return false;

    return true;
}

char *cftpclient::response()
{
    if (m_ftpconn == 0) return 0;

    return FtpLastResponse(m_ftpconn);
}

} // end namespace idc