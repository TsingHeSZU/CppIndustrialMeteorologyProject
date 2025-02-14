# 开发框架头文件路径
PUBINCL = -I/CppIndustrialMeteorologyProject/public

# 开发框架cpp文件名，这里直接包含进来，没有采用链接库，是为了方便调试
PUBCPP = /CppIndustrialMeteorologyProject/public/_public.cpp

# oracle头文件路径
ORAINCL = -I$(ORACLE_HOME)/rdbms/public -I/CppIndustrialMeteorologyProject/public/db/oracle

# oracle库文件路径
ORALIB =  -L$(ORACLE_HOME)/lib -L.

# oracle的oci库
ORALIBS = -lclntsh

# 开发框架 oracle 的 cpp 文件名，这里直接包含进来，没有采用链接库，是为了方便调试
ORACPP = /CppIndustrialMeteorologyProject/public/db/oracle/_ooci.cpp


# 编译参数。
CFLAGS = -g

all:crtsurfdata obtcode_to_db

crtsurfdata:crtsurfdata.cpp
	g++ $(CFLAGS) -o crtsurfdata crtsurfdata.cpp $(PUBINCL) $(PUBCPP)
	cp -f crtsurfdata ../bin/.

obtcode_to_db:obtcode_to_db.cpp
	g++ $(CFLAGS) -o obtcode_to_db obtcode_to_db.cpp $(PUBINCL) $(PUBCPP) $(ORAINCL) $(ORALIB) $(ORALIBS) $(ORACPP)
	cp -f obtcode_to_db ../bin/.

clean:
	rm -f crtsurfdata obtcode_to_db
