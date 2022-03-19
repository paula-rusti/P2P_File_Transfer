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

    if (request->header->message_type == REQUEST && request->header->message_subtype == VIEW_FILE_LIST )
    {
        handle_view_file_list(socket_fd);
    }
    else if(request->header->message_type == REQUEST && request->header->message_subtype == DOWNLOAD_FILE)
    {   
        printf("in tracker: calling handler for download file\n");
        //extract md5 from request body
        byte_t md5[16];
        memcpy(md5, request->body, 16);
        handle_download_file(socket_fd, md5);
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
    if (request->header->message_type == REQUEST && request->header->message_subtype == IS_FILE_PRESENT )
    {
        //extract md5 from request body
        byte_t md5[16];
        memcpy(md5, request->body, 16);
        handle_is_file_present(socket_fd, md5);
    }


}

void handle_transfer_bytes(int socket_fd)
{
    
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

void handle_download_file(int socket_fd, byte_t md5[16])    //received by the tracker
{
    printf("!!!!in handle_download_file \n");
    // broadcast to all peers to see if they have the file
    byte_t index_array[NODES_NR];    //sth like a freq vector
    int peers_nr = 0;   //nr of peers having the file
    for (int i=1; i<NODES_NR; i++) // iterate from 1 cuz the first node is the tracker
    {
        printf("!!!!in handle_download_file  index %d\n", i);
        int peer_fd = connect_to_node(NODES_ARRAY[i].ip_addr, NODES_ARRAY[i].port);
        if(peer_fd==-1)
            continue;

        message_t *message = message_constructor_from_params(REQUEST, IS_FILE_PRESENT, '0', 16, (byte_t*)md5);
        byte_t *serialized_message = serialize_message(message);
        
        int sent_bytes = write(peer_fd, serialized_message, (message->header->body_size+12));
        if (sent_bytes < 0)
        {
            printf("unable to request IS_FILE_PRESENT to peer %s %d", NODES_ARRAY[i].ip_addr, NODES_ARRAY[i].port);
        }

        byte_t raw_response[256];
        int read_bytes = read(peer_fd, raw_response, 256);
    
        if (read_bytes < 0)
        {
            printf("unable to read response!\n");
        }
        message_t *response = message_constructor_from_raw(raw_response);
        if (response->header->response_code==FILE_PRESENT)
        {
            index_array[i]='1';
            peers_nr++;
        }
        else
            index_array[i]='0';
    }
    printf("broadcast done!!!!!!!!!!\n");
    //boradcast sent, not we send to the client the array of peers having the file
    message_t *message = message_constructor_from_params(RESPONSE, DOWNLOAD_FILE, SUCCESS, NODES_NR, index_array);
    byte_t *serialized_message = serialize_message(message);
    
    int sent_bytes = write(socket_fd, serialized_message, (message->header->body_size+12));
    if (sent_bytes < 0)
    {
        printf("unable to send response from download file to peer client");
    }
}

void handle_is_file_present(int socket_fd, byte_t md5[16])   //received by peer servers
{
    //check to see if this peer has the requested md5
    printf("!!!in handle_is_file_present requested md5 is %16s", md5);
    int peer_has = 0;
    for (int i=0; i<4; i++)
    {
        if(strncmp((const char *)md5, (const char *)TEST_FILE_LIST[i].hash, 16)==0)
        {
            message_t *message = message_constructor_from_params(RESPONSE, IS_FILE_PRESENT, FILE_PRESENT, 0, NULL);
            byte_t *serialized_message = serialize_message(message);
            printf("in handle file is present : \n");
            print_message(message);
            
            int sent_bytes = write(socket_fd, serialized_message, (message->header->body_size+12));
            if (sent_bytes < 0)
            {
                printf("unable to request IS_FILE_PRESENT to peer %s %d", NODES_ARRAY[i].ip_addr, NODES_ARRAY[i].port);
            }
            peer_has = 1;
            break;
        }
        
    }
    if (!peer_has)
        {
            message_t *message = message_constructor_from_params(RESPONSE, IS_FILE_PRESENT, FILE_NOT_PRESENT, 0, NULL);
            byte_t *serialized_message = serialize_message(message);
            printf("in handle file is present : \n");
            print_message(message);
            
            int sent_bytes = write(socket_fd, serialized_message, (message->header->body_size+12));
            if (sent_bytes < 0)
            {
                perror("unable to request IS_FILE_PRESENT to peer");
            }
        }
}


void handle_send_file_list(int socket_fd)   //received by peer servers
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