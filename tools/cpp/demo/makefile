# 开发框架头文件路径
PUBINCL = -I/CppIndustrialMeteorologyProject/public/

# 开发框架 cpp 文件名，直接和程序的源代码文件一起编译，没有采用链接库，是为了方便调试
PUBCPP = /CppIndustrialMeteorologyProject/public/_public.cpp

# 编译选项
CFLAGS = -g

all: demo01 demo02
demo01: demo01.cpp
	g++ $(CFLAGS) demo01.cpp -o demo01 $(PUBINCL) $(PUBCPP)

demo02: demo02.cpp
	g++ $(CFLAGS) demo02.cpp -o demo02 $(PUBINCL) $(PUBCPP)