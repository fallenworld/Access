server:access_server
client:access_client
debug:access_server access_client
all:access_server access_client

access_server:access_server.c
	cc access_server.c -O2 -pthread -o access_server
access_client:access_client.c
	cc access_client.c -O2 -o access_client
clean:
	rm access_client access_server
