CFLAGS+=-Wall
server: server.c
	gcc ${CFLAGS} -o c_server server.c 

# utils: utils.c
# 	gcc ${CFLAGS} -o utils utils.c 

server_conc: server_conc.c
		gcc ${CFLAGS} -o server_conc server_conc.c 

clean:
	rm *.o
