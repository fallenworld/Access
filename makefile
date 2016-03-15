all:access_server access_client
server:access_server
client:access_client
debug:access_server access_client

access_server:access_server.c
	cc access_server.c -O2 -pthread -o access_server
access_client:access_client.c
	cc access_client.c -O2 -o access_client -D TARGET_RASPBERRY
access_client_debug:access_client.c
	cc access_client.c -o access_client
clean:
	rm -f access_client access_server
