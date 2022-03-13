#include "handlers.h"
#include "message.h"
#include "network_utils.h"
#include <stdio.h>
#include <unistd.h>

node_t NODES_ARRAY[];
file_t TEST_FILE_LIST[];

void handle_request_tracker(message_t *request, int socket_fd)
{   
    printf("TRACKER HANDLER!!!!!!!!!   \n");
    printf("msg is \n");
    print_message(request);

    if (request->header->message_type == '0' && request->header->message_subtype == VIEW_FILE_LIST )
    {
        handle_view_file_list(socket_fd);
    }
}

void handle_request_peer(message_t *request, int socket_fd)
{
    printf("PEER SERVER HANDLER!!!!!!!!!   \n");
    printf("msg is \n");
    print_message(request);

    if (request->header->message_type == '0' && request->header->message_subtype == SEND_FILE_LIST )
    {
        handle_send_file_list(socket_fd);
    }
}

void handle_view_file_list(int socket_fd)
{
    printf("in handle_view_file_list\n");
    //broadcast a SEND_FILE_LIST to all the peers in the network, read their responses and build the file names list, finally send that list to the client using fd
    for (int i=2; i<NODES_NR; i++) // iterate from 1 cuz the first node is the tracker
    {
        int peer_fd = connect_to_node(NODES_ARRAY[i].ip_addr, NODES_ARRAY[i].port);
        if(peer_fd==-1)
            continue;

        message_t *message = message_constructor_from_params('0', SEND_FILE_LIST, '0', 0, NULL);
        byte_t *serialized_message = serialize_message(message);
        
        int sent_bytes = write(peer_fd, serialized_message, (message->header->body_size+12));
        if (sent_bytes < 0)
        {
            printf("unable to request SEND_FILE_LIST to peer %s %d", NODES_ARRAY[i].ip_addr, NODES_ARRAY[i].port);
        }

        byte_t buffer[1024];
        int read_bytes = read(peer_fd, buffer, 1024);
        message_t *peer_response = message_constructor_from_raw(buffer);
        printf("peer response in fct handle view file list \n");
        print_message(peer_response);
        file_t *temp_file_list = deserialize_file_array(peer_response->body, peer_response->header->body_size);
        printf("first file name extracted: \n%s\n%s\n", temp_file_list[0].hash, temp_file_list[0].name);
    }
}

void handle_send_file_list(int socket_fd)
{
    printf("in handle_send_file_list\n");
    byte_t *body = serialize_file_array(TEST_FILE_LIST, 2);
    message_t * message = message_constructor_from_params('1', SEND_FILE_LIST, '0', 2*sizeof(file_t), body);
    byte_t *serialized_message = serialize_message(message);
    printf("mes to be sent in hanfle send file list: \n");
    print_message(message);
    int sent_bytes = write(socket_fd, serialized_message, (message->header->body_size+12));
    if (sent_bytes < 0)
    {
        printf("unable to send response to request SEND_FILE_LIST ");
    }
}