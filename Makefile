CFLAGS+=-Wall
server: server.c
	gcc ${CFLAGS} -o c_server server.c && ./c_server

clean:
	rm *.o
