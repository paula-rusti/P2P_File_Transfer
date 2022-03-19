#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <netdb.h>
#include <netinet/in.h>

#include "message.h"
#include "handlers.h"
#include "network_utils.h"
#include "file_utils.h"

typedef struct 
{
    struct offset off;
    char ip[64];
    int port;
}thread_param_t;

typedef struct
{
	thread_param_t off;
	pthread_t tid;
}thread_descriptor_t;

void *thread_job(void *arg)
{
    thread_param_t param = *(thread_param_t*)(arg);
    int new_socket_fd = connect_to_node(param.ip, param.port);  //connect to the server process of the peer having the file, to issue a transfer bytes request
    printf("In thread, params: ip: %s ---- port: %d\n\n", param.ip, param.port);

    byte_t body[sizeof(struct offset)];
    message_t *transfer_req = message_constructor_from_params(REQUEST, TRANSFER_BYTES, '0', sizeof(struct offset), body);
    byte_t *serialized = serialize_message(transfer_req);

    int sent_bytes = write(new_socket_fd, serialized, (transfer_req->header->body_size+12));
    if (sent_bytes < 0)
    {
        printf("unable to send msg to the server\n");
    }

    //read in blocks from the connection
    int read_bytes = 0;
    char buffer[4096];
    printf("THREAD X read bytes:\n");
    while((read_bytes = read(new_socket_fd, buffer, 4096))>0)
    {
        for (int i=0; i<read_bytes; i++)
        {
            putchar(buffer[i]);
        }
    }
    /*
    message_t
    write(new_socket_fd, request(transfer))

    read(new_socket_fd, buff, 4096)
    write(temp_file, buff, 4096)
    */

    return NULL;
}

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
    printf("connected, pls enter a message: \n");
    bzero(buffer, sizeof(buffer));
    fgets(buffer, sizeof(buffer), stdin);   //get the msg in a buffer -> msg body

    //build msg then serialize then send to the server
    message_t *message = message_constructor_from_params(REQUEST, DOWNLOAD_FILE, '0', sizeof(buffer), (byte_t*)buffer);
    byte_t *serialized_message = serialize_message(message);
    
    int sent_bytes = write(socket_fd, serialized_message, (message->header->body_size+12));
    if (sent_bytes < 0)
    {
        printf("unable to send msg to the server\n");
    }

    //also read the response from the tracker server in order to display the data on the screen
    byte_t raw_response[256];
    int read_bytes = read(socket_fd, raw_response, 256);

    if (read_bytes < 0)
    {
        printf("unable to read response!\n");
    }
    message_t *response = message_constructor_from_raw(raw_response);

    printf("GOT LIST OF PEERS:\n");
    print_message(response);

    node_t seeders[NODES_NR - 1]; // one node is always the tracker, hence the -1
    int seeders_index = 0;
    
    for (int i = 0; i <  response->header->body_size; i++)
    {
        if (response->body[i] == '1')
        {
            seeders[seeders_index++] = NODES_ARRAY[i];
        }
    }

    thread_descriptor_t threads[seeders_index];
    thread_param_t params[seeders_index];

    int file_size = 20; // bytes
    struct offset offsets[seeders_index];
    offsets[0].start = 0;
    offsets[0].end = 10;
    offsets[0].file_name = malloc(10);
    strncpy(offsets[0].file_name, "test_file", 10); 

    offsets[1].start = 11;
    offsets[1].end = 20;
    offsets[1].file_name = malloc(10);
    strncpy(offsets[1].file_name, "test_file", 10);

    for (int i = 0; i < seeders_index; i++)
    {
        params[i].off = offsets[i];
        strncpy(params[i].ip, seeders[i].ip_addr, strlen(seeders[i].ip_addr)+1);
        params[i].port = seeders[i].port;
    }

    for (int i = 0; i < seeders_index; i++)
    {
        printf("ip: %s ---- port: %d\n\n", seeders[i].ip_addr, seeders[i].port);

        //create thread
        if(pthread_create(&threads[i].tid, NULL, thread_job, &params[i]) != 0)
		{
			fprintf(stderr, "Thread creation failed.\n");
			return 1;
		}
    }
    
    for(int i = 0; i < seeders_index; i++)
	{
		if(pthread_join(threads[i].tid, NULL) != 0)
		{
			fprintf(stderr, "Thread join failed.\n");
			return 1;
		}
	}
    
    // reconstruct(temp_files)

    close(socket_fd);

    return 0;
}
