sources := message.c handlers.c network_utils.c server.c utils.c

all: peer_server.x test_server.x client.x peer_server2.x 

peer_server.x: 
	gcc -Wall -o peer_server.x $(sources) peer_server.c

test_server.x:
	gcc -Wall -o test_server.x $(sources) test_server.c

peer_server2.x:
	gcc -Wall -o peer_server2.x $(sources) peer_server2.c

client.x:
	gcc -Wall -o client.x $(sources) client.c

remove:
	rm *.x