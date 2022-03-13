#include "network_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>

#include "message.h"
#include "handlers.h"

node_t NODES_ARRAY[NODES_NR] = {
    {"localhost", 5010},     // tracker server
    {"localhost", 5001},     // peer 1 - server
    {"localhost", 5002}      // peer 2 - server
};

//on success returns an open file descriptor that can be used to write or read from the connection; else -1
//the fd must be closed after the transaction; e.g. after the function call
int connect_to_node(char *ip_addr, int port)
{
    int socket_fd;
    struct sockaddr_in server_addr;
    struct hostent *server;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0); //get a socket file descripter for a IPv4, TCP socket
    if (socket_fd < 0)
    {
        printf("error opening socket");
        return -1;
    }

    server = gethostbyname(ip_addr);
    if (server==NULL)
    {
        printf("no such host");
        return -1;
    }

    bzero( (char *)&server_addr, sizeof(server_addr) );
    server_addr.sin_family = AF_INET;
    bcopy( (char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length ); //copy the address of the server in the coresp field of the server_arr struct
    server_addr.sin_port=htons(port);    //add the port to the server address

    if ( connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr) ) < 0 )
    {
        printf("error in CONNECT");
        return -1;
    }
    return socket_fd;
}