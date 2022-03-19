sources := message.c handlers.c network_utils.c server.c utils.c

all: peer_server1.x peer_server2.x tracker.x client.x  

peer_server1.x: 
	gcc -Wall -o peer_server1.x $(sources) peer_server.c

tracker.x:
	gcc -Wall -o tracker.x $(sources) test_server.c

peer_server2.x:
	gcc -Wall -o peer_server2.x $(sources) peer_server2.c

client.x:
	gcc -Wall -o client.x $(sources) client.c

remove:
	rm *.x