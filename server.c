#include "server.h"
#include "message.h"
#include "handlers.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>  // debug  


void segment_file(int file_name, unsigned int nr_of_peers) {

    unsigned int nr_of_bytes;
    struct stat sb; //only for the member st_size

    if (fstat(file_name, &sb) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    } 

    nr_of_bytes = (unsigned int) sb.st_size / nr_of_peers;
    unsigned int rest = sb.st_size - (nr_of_bytes * nr_of_peers);
    char all[sb.st_size+1];

    printf("File size: %lu bytes\n", sb.st_size);
    printf("No peers: %d peers\n", nr_of_peers);
    printf("No bytes/peer: %d peers\n", nr_of_bytes);
    printf("No bytes rest: %d bytes\n", rest);

    unsigned int read_bytes;
    bool is_first_read = true;

    int file_name_out = open("outfile.txt", O_WRONLY | O_ASYNC, S_IRWXU);
    int curr_file_offset;

    for(int i = 0; i < nr_of_peers; i++) {
        if(is_first_read) {    //add rest of data to the first peer
            int first_time_req = nr_of_bytes + rest;
            char buf[first_time_req + 1];
            is_first_read = false;
            read_bytes = read(file_name, buf, first_time_req);
            
            printf("Read %d, bytes_req(%d), bytes_read(%d), buf_size(%u)\n", i, first_time_req, read_bytes, nr_of_bytes);
            
            lseek(file_name_out, 0, SEEK_SET);
            //here we should call an encryption function or a hash function in order not to send clear data over the network
            write(file_name_out, buf, first_time_req);
            curr_file_offset = read_bytes;
        }
        else {
            char buf[nr_of_bytes + 1];
            read_bytes = read(file_name, buf, nr_of_bytes);

            printf("Read %d, bytes_req(%d), bytes_read(%d), buf_size(%u)\n", i, nr_of_bytes, read_bytes, nr_of_bytes);
            
            lseek(file_name_out, curr_file_offset, SEEK_SET);
            write(file_name_out, buf, nr_of_bytes);
            curr_file_offset += nr_of_bytes;
        }
    }
    close(file_name);
    close(file_name_out);
}


server_t *server_constructor(char *ip_addr, int port, int backlog)
{
    printf("server constructor called!!!\n");
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
    printf("constructor done");
    return server;
}

void start_listening(server_t *server, void (*request_handler)(message_t *request))
{
    if (!server)
    {
        perror("probably server constructor failed");
        exit(2);
    }
    int inbound_conn_fd; //file descriptor corresponding to the socket opened for the inbound connection
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(struct sockaddr_in);
    printf("Will start listening\n");
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
            (*request_handler)(request);
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