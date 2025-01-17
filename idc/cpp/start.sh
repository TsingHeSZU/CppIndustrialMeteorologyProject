# 此脚本用于启动数据共享平台全部的服务程序

# 启动守护模块，终止心跳超时的进程，程序每 10s 运行一次
#/CppIndustrialMeteorologyProject/tools/bin/procctl 10 /CppIndustrialMeteorologyProject/tools/bin/checkproc /tmp/log/checkproc.log

# 生成气象站点观测的分钟数据，程序每 60s 运行一次
/CppIndustrialMeteorologyProject/tools/bin/procctl 60 /CppIndustrialMeteorologyProject/idc/bin/crtsurfdata /CppIndustrialMeteorologyProject/idc/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log csv,xml,json

# 清理气象观测数据目录（/tmp/idc/surfdata）中的历史文件，程序每 300s 运行一次, 0.01 天大致是 15 分钟之前的文件
/CppIndustrialMeteorologyProject/tools/bin/procctl 300 /CppIndustrialMeteorologyProject/tools/bin/deletefiles /tmp/idc/surfdata "*" 0.01

# 压缩后台服务程序的备份日志，程序每 300s 运行一次, 0.02 天大致是 30 分钟之前的文件
/CppIndustrialMeteorologyProject/tools/bin/procctl 300 /CppIndustrialMeteorologyProject/tools/bin/gzipfiles /log/idc "*.log.20*" 0.02

# 从 /tmp/idc/surfdata 目录下载原始的气象观测数据文件，存放在/idcdata/surfdata目录。
/CppIndustrialMeteorologyProject/tools/bin/procctl 30 /CppIndustrialMeteorologyProject/tools/bin/ftpgetfiles /log/idc/ftpgetfiles_surfdata.log "<host>127.0.0.1:21</host><mode>1</mode><username>root</username><password>Hq17373546038</password><localpath>/idcdata/surfdata</localpath><remotepath>/tmp/idc/surfdata</remotepath><matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname><listfilename>/idcdata/ftplist/ftpgetfiles_surfdata.list</listfilename><ptype>1</ptype><okfilename>/idcdata/ftplist/ftpgetfiles_surfdata.xml</okfilename><checkmtime>true</checkmtime><timeout>80</timeout><pname>ftpgetfiles_surfdata</pname>"

# 清理/idcdata/surfdata目录中0.04天之前的文件。
/CppIndustrialMeteorologyProject/tools/bin/procctl 300 /CppIndustrialMeteorologyProject/tools/bin/deletefiles /idcdata/surfdata "*" 0.04

# 把 /tmp/idc/surfdata 目录的原始气象观测数据文件上传到 /tmp/ftpputtest 目录。
# 注意，先创建好服务端的目录：mkdir /tmp/ftpputtest 
/CppIndustrialMeteorologyProject/tools/bin/procctl 30 /CppIndustrialMeteorologyProject/tools/bin/ftpputfiles /log/idc/ftpputfiles_surfdata.log "<host>127.0.0.1:21</host><mode>1</mode><username>root</username><password>Hq17373546038</password><localpath>/tmp/idc/surfdata</localpath><remotepath>/tmp/ftpputtest</remotepath><matchname>SURF_ZH*.JSON</matchname><ptype>1</ptype><okfilename>/idcdata/ftplist/ftpputfiles_surfdata.xml</okfilename><timeout>80</timeout><pname>ftpputfiles_surfdata</pname>"

# 清理 /tmp/ftpputtest 目录中 0.04 天之前的文件。
/CppIndustrialMeteorologyProject/tools/bin/procctl 300 /CppIndustrialMeteorologyProject/tools/bin/deletefiles /tmp/ftpputtest "*" 0.04