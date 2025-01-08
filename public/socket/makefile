all:client tcpselect tcppoll tcpepoll client1 tcpepoll1 tcpepoll2 client2 client3 client4

client1:client1.cpp
	g++ -g -o client1 client1.cpp -lm -lc

client2:client2.cpp
	g++ -g -o client2 client2.cpp -lm -lc

client3:client3.cpp
	g++ -g -o client3 client3.cpp -lm -lc

client4:client4.cpp
	g++ -g -o client4 client4.cpp -lm -lc

client:client.cpp
	g++ -g -o client client.cpp ../_public.cpp -lm -lc

tcpselect:tcpselect.cpp
	g++ -g -o tcpselect tcpselect.cpp ../_public.cpp -lm -lc

tcppoll:tcppoll.cpp
	g++ -g -o tcppoll tcppoll.cpp ../_public.cpp -lm -lc

tcpepoll:tcpepoll.cpp
	g++ -g -o tcpepoll tcpepoll.cpp ../_public.cpp -lm -lc

tcpepoll1:tcpepoll1.cpp
	g++ -g -o tcpepoll1 tcpepoll1.cpp ../_public.cpp -lm -lc

tcpepoll2:tcpepoll2.cpp
	g++ -g -o tcpepoll2 tcpepoll2.cpp ../_public.cpp -lm -lc

clean:
	rm -f client tcpselect tcppoll tcpepoll client1 tcpepoll1 tcpepoll2 client2 client3 client4
