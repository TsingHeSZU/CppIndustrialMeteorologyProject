# 此脚本用于启动数据共享平台全部的服务程序

# 启动守护模块
/CppIndustrialMeteorologyProject/tools/bin/procctl 10 /CppIndustrialMeteorologyProject/tools/bin/checkproc /tmp/log/checkproc.log

# 生成气象站点观测的分钟数据，程序每分钟运行一次
/CppIndustrialMeteorologyProject/tools/bin/procctl 600 /CppIndustrialMeteorologyProject/idc/bin/crtsurfdata /CppIndustrialMeteorologyProject/idc/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log csv,xml,json
