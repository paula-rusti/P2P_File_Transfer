#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>

#include "message.h"

int main(int argc, char *argv[])
{
    int socket_fd, port_nr;
    char buffer[256];
    struct sockaddr_in server_addr;
    struct hostent *server;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0); //get a socket file descripter for a IPv4, TCP socket
    if (socket_fd < 0)
    {
        printf("error opening socket");
        exit(1);
    }

    port_nr = 5001;
    server = gethostbyname("localhost");
    if (server==NULL)
    {
        printf("no such host");
        exit(1);
    }

    bzero( (char *)&server_addr, sizeof(server_addr) );
    server_addr.sin_family = AF_INET;
    bcopy( (char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length ); //copy the address of the server in the coresp field of the server_arr struct
    server_addr.sin_port=htons(port_nr);    //add the port to the server address

    //now try establishing connection
    if ( connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr) ) < 0 )
    {
        printf("error in CONNECT");
        exit(1);
    }
    //here we have connection
    printf("connected, pls enter a message: ");
    bzero(buffer, sizeof(buffer));
    fgets(buffer, sizeof(buffer), stdin);   //get the msg in a buffer

    //send the msg to the server
    header_t* header = header_constructor1('0', '1', '6', sizeof(buffer));
    byte_t *serialized = serialize_header(header);

    byte_t *raw_message = malloc(100);
    memcpy(raw_message, serialized, 12);
    memcpy(raw_message+12, buffer, sizeof(buffer));
    
    int sent_bytes = write(socket_fd, raw_message, (header->body_size+12));
    if (sent_bytes < 0)
    {
        printf("unable to send msg to the server");
    }

    return 0;
}