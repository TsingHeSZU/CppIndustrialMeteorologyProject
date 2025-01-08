#########################################
# 请注意，编译时，确定以下几点：
# 1、已安装oracle的客户端或oracle服务端程序，并且必须安装好OCI软件包
# 2、设置好ORACLE_HOME环境变量
#########################################

# oracle头文件路径
ORAINCL = -I$(ORACLE_HOME)/rdbms/public

# oracle库文件路径
ORALIB =  -L$(ORACLE_HOME)/lib -L.

# oracle的oci库
ORALIBS = -lclntsh

#CFLAGS = -O2
#CFLAGS = -O2 -Wall
CFLAGS = -g -Wall -Wno-write-strings #-Wno-unused-variable

all:    createtable inserttable updatetable deletetable selecttable filetoblob blobtofile\
         filetoclob clobtofile demo1 demo2 demo3 demo4

createtable: createtable.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o createtable createtable.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

inserttable: inserttable.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o inserttable inserttable.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

updatetable: updatetable.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o updatetable updatetable.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

deletetable: deletetable.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o deletetable deletetable.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

selecttable: selecttable.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o selecttable selecttable.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

filetoblob: filetoblob.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o filetoblob filetoblob.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

blobtofile: blobtofile.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o blobtofile blobtofile.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

filetoclob: filetoclob.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o filetoclob filetoclob.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

clobtofile: clobtofile.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o clobtofile clobtofile.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

demo1: demo1.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o demo1 demo1.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

demo2: demo2.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o demo2 demo2.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

demo3: demo3.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o demo3 demo3.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

demo4: demo4.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o demo4 demo4.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

demo5: demo5.cpp _ooci.h _ooci.cpp
	g++ $(CFLAGS) -o demo5 demo5.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _ooci.cpp 

clean:
	rm -rf createtable inserttable updatetable deletetable selecttable filetoblob blobtofile
	rm -rf filetoclob clobtofile demo1 demo2 demo3 demo4
