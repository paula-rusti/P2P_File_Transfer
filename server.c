#include "server.h"
#include "message.h"
#include "handlers.h"
#include "message.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>  // debug  


void start_listening(server_t*, void (*request_handler)(message_t *request, int socket_fd));

server_t *server_constructor(char *ip_addr, int port, int backlog)
{
    //printf("server constructor called!!!\n");
    server_t *server = malloc(sizeof(server_t));
    
    server->start_listening = start_listening;

    server->port = port;
    server->backlog = backlog;
 
    server->address.sin_family = AF_INET;                       // ipv4
    server->address.sin_addr.s_addr = inet_addr(ip_addr);       // the given ip addr
    server->address.sin_port = htons(server->port);             // the given port

    server->socket = socket(AF_INET, SOCK_STREAM, 0); //get a socket file descripter for a IPv4, TCP socket
    
    // Confirm the connection was successful.
    if (server->socket == -1)
    {
        perror("Failed to open socket...\n");
        exit(1);
    }
    // Attempt to bind the socket to the network.
    if ((bind(server->socket, (struct sockaddr *)&server->address, sizeof(server->address))) == -1)
    {
        perror("Failed to bind socket...\n");
        exit(1);
    }
    // Start listening on the network, only sets some flags
    if ((listen(server->socket, server->backlog)) == -1)
    {
        perror("Failed to start listening...\n");
        exit(1);
    }
    //printf("constructor done");
    return server;
}

void start_listening(server_t *server, void (*request_handler)(message_t *request, int socket_fd))
{
    if (!server)
    {
        perror("probably server constructor failed");
        exit(2);
    }
    int inbound_conn_fd; //file descriptor corresponding to the socket opened for the inbound connection
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(struct sockaddr_in);
    printf("Listening...\n");
    for (;;)
    {
        //accept incoming connections, this call blocks until a peer connects
        inbound_conn_fd = accept(server->socket, (struct sockaddr*)&client_addr, (unsigned *)&client_addr_len);
        if (inbound_conn_fd == -1)
        {
            printf("error on accept");
            exit(1);
        }
        // now create a new process to handle this request
        int pid = fork();
        if (pid < 0)
        {
            printf("error creating a new process");
            exit(1);
        }
        if (pid==0) // child process
        {
            printf("child process\n");
            close(server->socket);    //THE CHILD PROCESS ALWAYS CLOSES THIS FD
            // read bytes from inbound_conn_fd(the client process of a peer) and try to assemble them in a valid message of type request
            byte_t *buffer = malloc(BYTES_SIZE_IN_LISTEN*sizeof(byte_t));
            read_request_message(inbound_conn_fd, buffer); //here buffer contains the request message
            message_t *request = message_constructor_from_raw(buffer);  //this perform the serialization as well
            (*request_handler)(request, inbound_conn_fd);
            close(inbound_conn_fd);     // close fb after reading and handling the request
            exit(0);
        }
        else //parent process
        {
            close(inbound_conn_fd);
        }
    }
}

void read_request_message(int fd, byte_t *buffer)
{
    bzero(buffer, BYTES_SIZE_IN_LISTEN);    // init buff with 0's
    int n = read(fd, buffer, BYTES_SIZE_IN_LISTEN);
    if (n < 0)
    {
        printf("ERROR reading bytes of a request message");
        exit(1);
    }
}