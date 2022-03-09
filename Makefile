CFLAGS+=-Wall
server: server.c
	gcc ${CFLAGS} -o c_server server.c

clean:
	rm *.o
