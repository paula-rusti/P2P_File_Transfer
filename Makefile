CFLAGS+=-Wall
server: server.c
	gcc ${CFLAGS} -o c_server server.c 

server_conc: server_conc.c
		gcc ${CFLAGS} -o server_conc server_conc.c 

file_utils: file_utils.c
		gcc ${CFLAGS} -o file_utils file_utils.c

clean:
	rm *.o
