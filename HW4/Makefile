all:socks_server.cpp console.cpp
	g++ socks_server.cpp -o socks_server -std=c++14 -Wall -pedantic -pthread -lboost_system
	g++ console.cpp -o hw4.cgi -std=c++14 -Wall -pedantic -pthread -lboost_system
hw3:http_server.cpp 
	g++ http_server.cpp -o http_server -std=c++14 -Wall -pedantic -pthread -lboost_system
	g++ console.cpp -o console.cgi -std=c++14 -Wall -pedantic -pthread -lboost_system
clean:
	rm -f http_server console.cgi socks_server hw4.cgi
