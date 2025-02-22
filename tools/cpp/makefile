# 开发框架头文件路径
PUBINCL = -I/CppIndustrialMeteorologyProject/public/

# 开发框架 cpp 文件名，直接和程序的源代码文件一起编译，没有采用链接库，是为了方便调试
PUBCPP = /CppIndustrialMeteorologyProject/public/_public.cpp

# oracle头文件路径
ORAINCL = -I$(ORACLE_HOME)/rdbms/public -I/CppIndustrialMeteorologyProject/public/db/oracle

# oracle库文件路径
ORALIB =  -L$(ORACLE_HOME)/lib -L.

# oracle的oci库
ORALIBS = -lclntsh

# 开发框架 oracle 的 cpp 文件名，这里直接包含进来，没有采用链接库，是为了方便调试
ORACPP = /CppIndustrialMeteorologyProject/public/db/oracle/_ooci.cpp

# 编译选项
CFLAGS = -g
# CLFAGS = -O2

all: procctl checkproc deletefiles gzipfiles ftpgetfiles ftpputfiles tcpfileserver tcpputfiles\
	 tcpputfiles_io_multi tcpgetfiles dminingoracle xmltodb deletetable migratetable
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

tcpfileserver: tcpfileserver.cpp
	g++ $(CFLAGS) tcpfileserver.cpp -o tcpfileserver $(PUBINCL) $(PUBCPP)
	cp -f tcpfileserver ../bin/.

tcpputfiles: tcpputfiles.cpp
	g++ $(CFLAGS) tcpputfiles.cpp -o tcpputfiles $(PUBINCL) $(PUBCPP)
	cp -f tcpputfiles ../bin/.	

tcpputfiles_io_multi: tcpputfiles_io_multi.cpp
	g++ $(CFLAGS) tcpputfiles_io_multi.cpp -o tcpputfiles_io_multi $(PUBINCL) $(PUBCPP)
	cp -f tcpputfiles_io_multi ../bin/.

tcpgetfiles: tcpgetfiles.cpp
	g++ $(CFLAGS) tcpgetfiles.cpp -o tcpgetfiles $(PUBINCL) $(PUBCPP)
	cp -f tcpgetfiles ../bin/.	

dminingoracle: dminingoracle.cpp
	g++ $(CFLAGS) -o dminingoracle dminingoracle.cpp $(PUBINCL) $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP)
	cp -f dminingoracle ../bin/.

xmltodb:xmltodb.cpp _tools.cpp
	g++ $(CFLAGS) -o xmltodb xmltodb.cpp $(PUBINCL) $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP) _tools.cpp
	cp -f xmltodb ../bin/.	

deletetable:deletetable.cpp
	g++ $(CFLAGS) -o deletetable deletetable.cpp $(PUBINCL) $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP)
	cp -f deletetable ../bin/.	

migratetable:migratetable.cpp _tools.cpp
	g++ $(CFLAGS) -o migratetable migratetable.cpp $(PUBINCL) $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP) _tools.cpp
	cp -f migratetable ../bin/.

clean:
	rm -rf procctl checkproc deletefiles gzipfiles ftpgetfiles ftpputfiles 
	rm -rf tcpfileserver tcpputfiles tcpputfiles_io_multi tcpgetfiles dminingoracle
	rm -rf xmltodb deletetable migratetable