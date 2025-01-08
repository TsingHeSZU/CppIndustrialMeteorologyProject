# 开发框架头文件路径。
PUBINCL = -I/CppIndustrialMeteorologyProject/public

# 开发框架cpp文件名，这里直接包含进来，没有采用链接库，是为了方便调试。
PUBCPP = /CppIndustrialMeteorologyProject/public/_public.cpp

# 编译参数。
CFLAGS = -g

all:crtsurfdata

crtsurfdata:crtsurfdata.cpp
	g++ $(CFLAGS) -o crtsurfdata crtsurfdata.cpp $(PUBINCL) $(PUBCPP) -Im -Ic
	cp -f crtsurfdata ../bin/.

clean:
	rm -f crtsurfdata
