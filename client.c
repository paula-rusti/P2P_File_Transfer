#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>

#include "message.h"
#include "handlers.h"
#include "network_utils.h"


int main(int argc, char *argv[])
{
    int socket_fd = connect_to_node("localhost", 5010);

    if(socket_fd == -1)
    {
        printf("unable to connect to tracker");
        exit(1);
    }

    char buffer[256];
    //here we have connection
    printf("connected, pls enter a message: ");
    bzero(buffer, sizeof(buffer));
    fgets(buffer, sizeof(buffer), stdin);   //get the msg in a buffer -> msg body

    //build msg then serialize then send to the server
    message_t *message = message_constructor_from_params('0', VIEW_FILE_LIST, '0', sizeof(buffer), (byte_t*)buffer);
    byte_t *serialized_message = serialize_message(message);
    
    int sent_bytes = write(socket_fd, serialized_message, (message->header->body_size+12));
    if (sent_bytes < 0)
    {
        printf("unable to send msg to the server");
    }

    //also read the response from the tracker server in order to display the data on the screen

    close(socket_fd);

    return 0;
}