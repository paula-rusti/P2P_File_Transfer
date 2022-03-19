#include "handlers.h"
#include "message.h"
#include "network_utils.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
    file_t *files_arr = malloc(MAX_FILES_PER_PEER * NODES_NR * sizeof(file_t));
    int files_index = 0;

    for (int i=1; i<NODES_NR; i++) // iterate from 1 cuz the first node is the tracker
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

        // iterate though each file from peer response and add to complete list
        int files_number = peer_response->header->body_size / FILE_STRUCT_SIZE;

        for (int i = 0; i < files_number; i++)
        {   
            int file_in_array = 0;
            // check if file is not already in array to avoid duplicate files
            for (int j = 0; j < files_index; j++)
            {  
                if ((strncmp((const char *)temp_file_list[i].hash, (const char *)files_arr[j].hash, 16) == 0))
                {
                    file_in_array = 1;
                }
            }

            if (file_in_array == 0)
            {
                memcpy(files_arr[files_index].hash, temp_file_list[i].hash, 16);
                memcpy(files_arr[files_index].name, temp_file_list[i].name, 10);
                files_index++;
            }
        }

        close(peer_fd);
    }

    // build message for client
    byte_t *client_body = serialize_file_array(files_arr, files_index);
    message_t *for_client = message_constructor_from_params('1', VIEW_FILE_LIST, '0', files_index * FILE_STRUCT_SIZE, client_body);

    byte_t *for_client_raw = serialize_message(for_client);

    print_message(for_client);
    write(socket_fd, for_client_raw, HEADER_SIZE + for_client->header->body_size);
}

void handle_send_file_list(int socket_fd)
{
    printf("in handle_send_file_list\n");
    byte_t *body = serialize_file_array(TEST_FILE_LIST, 3);
    message_t * message = message_constructor_from_params('1', SEND_FILE_LIST, '0', 3*sizeof(file_t), body);
    byte_t *serialized_message = serialize_message(message);
    printf("mes to be sent in hanfle send file list: \n");
    print_message(message);
    int sent_bytes = write(socket_fd, serialized_message, (message->header->body_size+12));
    if (sent_bytes < 0)
    {
        printf("unable to send response to request SEND_FILE_LIST ");
    }
}