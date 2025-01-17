# 开发框架头文件路径
PUBINCL = -I/CppIndustrialMeteorologyProject/public/

# 开发框架 cpp 文件名，直接和程序的源代码文件一起编译，没有采用链接库，是为了方便调试
PUBCPP = /CppIndustrialMeteorologyProject/public/_public.cpp

# 编译选项
CFLAGS = -g
# CLFAGS = -O2

all: procctl checkproc deletefiles gzipfiles ftpgetfiles ftpputfiles
procctl: procctl.cpp
	g++ -g -o procctl procctl.cpp
	cp -f procctl ../bin/.

checkproc: checkproc.cpp
	g++ $(CFLAGS) checkproc.cpp -o checkproc $(PUBINCL) $(PUBCPP)
	cp -f checkproc ../bin/.

deletefiles: deletefiles.cpp
	g++ $(CFLAGS) deletefiles.cpp -o deletefiles $(PUBINCL) $(PUBCPP)
	cp -f deletefiles ../bin/.

gzipfiles: gzipfiles.cpp
	g++ $(CFLAGS) gzipfiles.cpp -o gzipfiles $(PUBINCL) $(PUBCPP)
	cp -f gzipfiles ../bin/.

ftpgetfiles: ftpgetfiles.cpp
	g++ $(CFLAGS) ftpgetfiles.cpp -o ftpgetfiles $(PUBINCL) $(PUBCPP) /CppIndustrialMeteorologyProject/public/libftp.a /CppIndustrialMeteorologyProject/public/_ftp.cpp
	cp -f ftpgetfiles ../bin/.

ftpputfiles: ftpputfiles.cpp
	g++ $(CFLAGS) ftpputfiles.cpp -o ftpputfiles $(PUBINCL) $(PUBCPP) /CppIndustrialMeteorologyProject/public/libftp.a /CppIndustrialMeteorologyProject/public/_ftp.cpp
	cp -f ftpputfiles ../bin/.

clean:
	rm -rf procctl checkproc deletefiles gzipfiles ftpgetfiles ftpputfiles