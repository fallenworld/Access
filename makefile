ver = debug
COMMAND_FLAGS =
cc = gcc

ifeq ($(ver), server)
all:access_server
else
ifeq ($(ver), client)
all:access_client
COMMANDS_FLAG = -D TARGET_RASPBERRY
else
all:access_server access_client
endif
endif


access_server:access_server.o access_helper.o
	$(cc) -o access_server access_server.o access_helper.o -pthread
access_client:access_client.o access_helper.o access_commands.o
	$(cc) -o access_client access_client.o access_helper.o access_commands.o

access_client.o:access_client.c 
	$(cc) -c access_client.c
access_server.o:access_server.c
	$(cc) -c access_server.c
access_helper.o:access_helper.c
	$(cc) -c access_helper.c
access_commands.o:
	$(cc) -c access_commands.c $(COMMANDS_FLAG)

.PHONY:clean
clean:
	rm -f *.o access_client access_server
