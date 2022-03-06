#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    int socket_fd, new_socket_fd, prot_nr, client_addr_len;
    char buffer[256];
    struct sockaddr_in server_addr, client_addr;
    int n;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0); //get a socket file descripter for a IPv4, TCP socket
    if (socket_fd < 0)
    {
        printf("error opening socket");
        exit(1);
    }

    bzero( (char *) &server_addr, sizeof(server_addr) ); //init server addr structure with bytes of 0
    prot_nr = 5001;
    server_addr.sin_family = AF_INET;   //ipv4
    server_addr.sin_addr.s_addr = INADDR_ANY;   //any addr
    server_addr.sin_port = htons(prot_nr);  //port 5001
    //bind our socket file descriptor to the server address
    if( bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 )
    {
        printf("error on binding");
        exit(1);
    }

    listen(socket_fd, 5); //sets a flag so we accept incoming connections to this socket
    client_addr_len = sizeof(client_addr);

    for (;;)
    {
        //accept incoming connections, this call blocks until sb connects
        new_socket_fd = accept(socket_fd, (struct sockaddr*)&client_addr, (unsigned *)&client_addr_len);
        if (new_socket_fd < 0)
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
            close(socket_fd);
            bzero(buffer,256);
            n = read(new_socket_fd, buffer,255 );
            if (n < 0)
            {
                printf("ERROR reading from socket");
                exit(1);
            }
            printf("Here is the message: %s\n",buffer);
            exit(0);
        }
        else //parent process
        {
            close(new_socket_fd);
        }

    
    }

    return 0;
}