#Compiling config, possible value:
#raspberry : compile on the raspberry
#server : compile on the server
#debug : debug mode, it will compile both the raspberry code and the server code
config = debug

ifeq ($(config), debug)
all:access_server access_client
else ifeq ($(config), server)
all:access_server
else ifeq ($(config), raspberry)
all:access_client
endif

access_server:access_server.c
	cc access_server.c -O2 -pthread -o access_server
access_client:access_client.c
	cc access_client.c-O2 -o access_client
clean:
	rm access_client access_server
