
.PHONY:clean

all:server client

server:server.cpp
	clang++ $^ -o $@ -g -lpthread

client:client.cpp
	clang++ -o $@  $^ -g 

clean:
	rm -f client server core.*
