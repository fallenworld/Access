all:access_server access_client
access_server:access_server.c
	cc access_server.c -o access_server
access_client:access_client.c
	cc access_client.c -o access_client
clean:
	rm access_client access_server
