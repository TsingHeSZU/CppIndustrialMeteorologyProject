# 此脚本用于启动数据共享平台全部的服务程序

# 启动守护模块，终止心跳超时的进程，程序每 10s 运行一次
/CppIndustrialMeteorologyProject/tools/bin/procctl 10 /CppIndustrialMeteorologyProject/tools/bin/checkproc /tmp/log/checkproc.log

# 生成气象站点观测的分钟数据，程序每 60s 运行一次
/CppIndustrialMeteorologyProject/tools/bin/procctl 60 /CppIndustrialMeteorologyProject/idc/bin/crtsurfdata /CppIndustrialMeteorologyProject/idc/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log csv,xml,json

# 清理气象观测数据目录（/tmp/idc/surfdata）中的历史文件，程序每 300s 运行一次, 0.01 天大致是 15 分钟之前的文件
/CppIndustrialMeteorologyProject/tools/bin/procctl 300 /CppIndustrialMeteorologyProject/tools/bin/deletefiles /tmp/idc/surfdata "*" 0.01

# 压缩后台服务程序的备份日志，程序每 300s 运行一次, 0.02 天大致是 30 分钟之前的文件
/CppIndustrialMeteorologyProject/tools/bin/procctl 300 /CppIndustrialMeteorologyProject/tools/bin/gzipfiles /log/idc "*.log.20*" 0.02