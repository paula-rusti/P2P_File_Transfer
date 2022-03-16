/* server.h */
#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "message.h"

#define BYTES_SIZE_IN_LISTEN 1024 // how many bytes to read from the client process of a peer in order to get a message

//struct only to store the start and end offset
struct offset{
    unsigned int start;
    unsigned int end;
};

// struct offset *
void segment_file(int file_name, unsigned int nr_of_peers);




typedef struct Server{
    char *ip_address; //string representing an ip
    int port;
    int backlog;
    struct sockaddr_in address; //socket address
    int socket;  //basically an open file descriptor
    void (*start_listening)(struct Server *server, void (*request_handler)(message_t *request)); //the fct which spawns new processes for every incoming request
}server_t;

server_t *server_constructor(char *ip_addr, int port, int backlog);

// util function for reading bytes from the connection in order to get a request message
void read_request_message(int fd, byte_t *buffer); //reads at most BYTES_SIZE_IN_LISTEN from the given fd and fills the buffer

#endif