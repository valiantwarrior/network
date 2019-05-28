all : server client

server : ChatTCPServer.c
	gcc -o server ChatTCPServer.c -lpthread

client : ChatTCPClient.c
	gcc -o client ChatTCPClient.c -lpthread

clean: 
	rm server
	rm client
