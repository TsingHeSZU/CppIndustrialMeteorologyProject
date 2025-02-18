# 此脚本用于停止数据共享平台全部的服务程序

# 停止调度程序
killall -9 procctl

# 停止其它服务程序
killall crtsurfdata deletefiles gzipfiles ftpgetfiles ftpputfiles
killall tcpfileserver tcpputfiles_io_multi tcpgetfiles obtmind_to_db
killall dminingoracle

# 让其它服务程序有足够时间退出
sleep 5

# 防止出现异常的程序不能正常退出，都强制杀死
killall -9 crtsurfdata deletefiles gzipfiles ftpgetfiles ftpputfiles
killall -9 tcpfileserver tcpputfiles_io_multi tcpgetfiles obtmind_to_db
killall -9 dminingoracle



