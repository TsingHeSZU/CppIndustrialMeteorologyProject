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

all: demo01 demo02 demo03 demo04 demo05 demo06 client server server1 server2
demo01: demo01.cpp
	g++ $(CFLAGS) demo01.cpp -o demo01 $(PUBINCL) $(PUBCPP)

demo02: demo02.cpp
	g++ $(CFLAGS) demo02.cpp -o demo02 $(PUBINCL) $(PUBCPP)

demo03: demo03.cpp
	g++ $(CFLAGS) demo03.cpp -o demo03 $(PUBINCL) $(PUBCPP)

demo04: demo04.cpp
	g++ $(CFLAGS) demo04.cpp -o demo04 $(PUBINCL) $(PUBCPP)

demo05: demo05.cpp
	g++ $(CFLAGS) demo05.cpp -o demo05 $(PUBINCL) $(PUBCPP)

demo06: demo06.cpp
	g++ $(CFLAGS) demo06.cpp -o demo06 $(PUBINCL) $(PUBCPP)

client: client.cpp
	g++ $(CFLAGS) client.cpp -o client $(PUBINCL) $(PUBCPP)

server: server.cpp
	g++ $(CFLAGS) server.cpp -o server $(PUBINCL) $(PUBCPP)

server1: server1.cpp
	g++ $(CFLAGS) server1.cpp -o server1 $(PUBINCL) $(PUBCPP)

server2: server2.cpp
	g++ $(CFLAGS) -o server2 server2.cpp $(PUBINCL) $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP)
