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
/CppIndustrialMeteorologyProject/tools/bin/procctl 30 /CppIndustrialMeteorologyProject/tools/bin/ftpgetfiles /log/idc/ftpgetfiles_surfdata.log "<host>127.0.0.1:21</host><mode>1</mode><username>utopianyouth</username><password>123</password><localpath>/idcdata/surfdata</localpath><remotepath>/tmp/idc/surfdata</remotepath><matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname><listfilename>/idcdata/ftplist/ftpgetfiles_surfdata.list</listfilename><ptype>1</ptype><okfilename>/idcdata/ftplist/ftpgetfiles_surfdata.xml</okfilename><checkmtime>true</checkmtime><timeout>80</timeout><pname>ftpgetfiles_surfdata</pname>"

# 清理/idcdata/surfdata目录中0.04天之前的文件。
/CppIndustrialMeteorologyProject/tools/bin/procctl 300 /CppIndustrialMeteorologyProject/tools/bin/deletefiles /idcdata/surfdata "*" 0.04

# 把 /tmp/idc/surfdata 目录的原始气象观测数据文件上传到 /tmp/ftpputtest 目录。
# 注意，先创建好服务端的目录：mkdir /tmp/ftpputtest 
/CppIndustrialMeteorologyProject/tools/bin/procctl 30 /CppIndustrialMeteorologyProject/tools/bin/ftpputfiles /log/idc/ftpputfiles_surfdata.log "<host>127.0.0.1:21</host><mode>1</mode><username>utopianyouth</username><password>123</password><localpath>/tmp/idc/surfdata</localpath><remotepath>/tmp/ftpputtest</remotepath><matchname>SURF_ZH*.JSON</matchname><ptype>1</ptype><okfilename>/idcdata/ftplist/ftpputfiles_surfdata.xml</okfilename><timeout>80</timeout><pname>ftpputfiles_surfdata</pname>"

# 清理 /tmp/ftpputtest 目录中 0.04 天之前的文件。
/CppIndustrialMeteorologyProject/tools/bin/procctl 300 /CppIndustrialMeteorologyProject/tools/bin/deletefiles /tmp/ftpputtest "*" 0.04

# 文件传输的服务端程序。
/CppIndustrialMeteorologyProject/tools/bin/procctl 10 /CppIndustrialMeteorologyProject/tools/bin/tcpfileserver 5005 /log/idc/tcpfileserver.log

# 把目录 /tmp/ftpputtest 中的文件上传到 /tmp/tcpputtest 目录中。
/CppIndustrialMeteorologyProject/tools/bin/procctl 20 /CppIndustrialMeteorologyProject/tools/bin/tcpputfiles_io_multi /log/idc/tcpputfiles_io_multi_surfdata.log "<clienttype>1</clienttype><ip>127.0.0.1</ip><port>5005</port><ptype>1</ptype><clientpath>/tmp/ftpputtest</clientpath><clientpathbak>/tmp/ftpputtesst_tcp_bak</clientpathbak><ischild>true</ischild><matchname>*.xml,*.txt,*.csv,*.json</matchname><serverpath>/tmp/tcpputtest</serverpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpputfiles_io_multi_surfdata</pname>"

# 把目录 /tmp/tcpputtest 中的文件下载到 /tmp/tcpgettest 目录中。
/CppIndustrialMeteorologyProject/tools/bin/procctl 20 /CppIndustrialMeteorologyProject/tools/bin/tcpgetfiles /log/idc/tcpgetfiles_surfdata.log "<clienttype>2</clienttype><ip>127.0.0.1</ip><port>5005</port><ptype>1</ptype><serverpath>/tmp/tcpputtest</serverpath><serverpathbak>/tmp/tcpputtest_tcp_bak</serverpathbak><ischild>true</ischild><clientpath>/tmp/tcpgettest</clientpath><matchname>*.xml,*.txt,*.csv,*.json</matchname><timetvl>10</timetvl><timeout>50</timeout><pname>tcpgetfiles_surfdata</pname>"

# 清理 /tmp/tcpgettest 目录中的历史数据文件。
/CppIndustrialMeteorologyProject/tools/bin/procctl 300 /CppIndustrialMeteorologyProject/tools/bin/deletefiles /tmp/tcpgettest "*" 0.02

# 把 /idcdata/surfdata 目录中的气象观测数据文件入库到 T_ZHOBTMIND 表中
/CppIndustrialMeteorologyProject/tools/bin/procctl 10 /CppIndustrialMeteorologyProject/idc/bin/obtmind_to_db /idcdata/surfdata "idc/idcpwd" "Simplified Chinese_China.AL32UTF8" /log/idc/obtmind_to_db.log

# 执行 /CppIndustrialMeteorologyProject/idc/sql/delete_table.sql 脚本，删除 T_ZHOBTMIND 表两小时之前的数据
/CppIndustrialMeteorologyProject/tools/bin/procctl 120 /oracle/home/bin/sqlplus idc/idcpwd @/CppIndustrialMeteorologyProject/idc/sql/deletetable.sql