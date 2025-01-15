all: lib_public.a lib_public.so libftp.a libftp.so

lib_public.a:_public.h _public.cpp
	g++ -c _public.cpp -o lib_public.a 

lib_public.so:_public.h _public.cpp
	g++ -fPIC -shared _public.cpp -o lib_public.so 

libftp.a:ftplib.h ftplib.c
	gcc -c ftplib.c -o libftp.a 

libftp.so:ftplib.h ftplib.c
	gcc -fPIC -shared ftplib.c -o libftp.so 

clean:
	rm -f lib_public.a lib_public.so libftp.a libftp.so
