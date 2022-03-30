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
#include "utils.h"

typedef struct
{
    struct offset off;
    char ip[64];
    int port;
} thread_param_t;

typedef struct
{
    thread_param_t off;
    pthread_t tid;
} thread_descriptor_t;

file_t *a_files;
unsigned a_files_number;

void *thread_job(void *arg)
{
    // get parameters nicely
    thread_param_t param = *(thread_param_t *)(arg);
    // connect to the server process of the peer having the file, to issue a transfer bytes request
    int new_socket_fd = connect_to_node(param.ip, param.port);
    printf("Spawned thread, getting file from peer: ip: %s ---- port: %d\n\n", param.ip, param.port);

    // file_name is variable in size and we cannot say sizeof(struct offset) because we have a char *, which is fixed in size
    unsigned body_size = strlen(param.off.file_name) + 2 * sizeof(int);

    byte_t *body = malloc(sizeof(byte_t) * body_size);

    // insert given offset struct into body
    memcpy(body, param.off.file_name, strlen(param.off.file_name));
    *(int *)(body + strlen(param.off.file_name)) = param.off.start;
    *(int *)(body + strlen(param.off.file_name) + 4) = param.off.end;

    message_t *transfer_req = message_constructor_from_params(REQUEST, TRANSFER_BYTES, '0', body_size, body);
    byte_t *serialized = serialize_message(transfer_req);

    // send TRANSFER_BYTES request to the other peer's server
    int sent_bytes = write(new_socket_fd, serialized, (transfer_req->header->body_size + HEADER_SIZE));
    if (sent_bytes < 0)
    {
        printf("unable to send msg to the server\n");
    }

    // read in blocks from the connection
    int read_bytes = 0;
    char buffer[4096];

    // name of the temp file for this segment
    char tmp_name[100];

    // build the temp file's name
    snprintf(tmp_name, 100, "%d_temp", param.off.start);

    // create temp file and write the segment
    int tmp_fd = open(tmp_name, O_WRONLY | O_CREAT);
    int written_bytes;

    while ((read_bytes = read(new_socket_fd, buffer, 4096)) > 0)
    {
        written_bytes = write(tmp_fd, buffer, read_bytes);
        if (written_bytes == -1)
        {
            printf("write() error in thread_job()\n");
        }
    }

    return NULL;
}

void download_file_routine(unsigned char *file_name, unsigned char *md5, int file_size)
{
    int socket_fd = connect_to_node("localhost", 5010);

    if (socket_fd == -1)
    {
        printf("unable to connect to tracker");
        exit(1);
    }

    // build msg then serialize then send to the server
    message_t *message = message_constructor_from_params(REQUEST, DOWNLOAD_FILE, '0', HASH_SIZE, (byte_t *)md5);
    byte_t *serialized_message = serialize_message(message);

    printf("Sending DOWNLOAD FILE request...\n");
    int sent_bytes = write(socket_fd, serialized_message, (message->header->body_size + HEADER_SIZE));
    if (sent_bytes < 0)
    {
        printf("unable to send msg to the server\n");
    }

    // also read the response from the tracker server in order to display the data on the screen
    byte_t raw_response[256];
    int read_bytes = read(socket_fd, raw_response, 256);

    if (read_bytes < 0)
    {
        printf("unable to read response!\n");
    }
    message_t *response = message_constructor_from_raw(raw_response);

    //printf("GOT LIST OF PEERS:\n");
    //print_message(response);

    node_t seeders[NODES_NR - 1]; // one node is always the tracker, hence the -1
    int seeders_index = 0;
    int nodes_arr_idx = 1;


    for (int i = 0; i < response->header->body_size; i++)
    {
        if (response->body[i] == '1')
        {
            seeders[seeders_index++] = NODES_ARRAY[nodes_arr_idx++];
        }
    }


    thread_descriptor_t threads[seeders_index];
    thread_param_t params[seeders_index];
    struct offset *offsets = segment_file_size(file_size, seeders_index);


    for (int i = 0; i < seeders_index; i++)
    {
        offsets[i].file_name = malloc(100);
        strncpy((char *)offsets[i].file_name, (const char *)file_name, NAME_SIZE);
        params[i].off = offsets[i];
        strncpy(params[i].ip, seeders[i].ip_addr, strlen(seeders[i].ip_addr) + 1);
        params[i].port = seeders[i].port;
    }


    for (int i = 0; i < seeders_index; i++)
    {
        //printf("ip: %s ---- port: %d\n\n", seeders[i].ip_addr, seeders[i].port);

        // create thread
        if (pthread_create(&threads[i].tid, NULL, thread_job, &params[i]) != 0)
        {
            fprintf(stderr, "Thread creation failed.\n");
            exit(1);
        }
    }

    for (int i = 0; i < seeders_index; i++)
    {
        if (pthread_join(threads[i].tid, NULL) != 0)
        {
            fprintf(stderr, "Thread join failed.\n");
            exit(1);
        }
    }

    reconstruct_file(offsets, seeders_index);

    close(socket_fd);
}

void view_file_list_routine()
{
    int socket_fd = connect_to_node("localhost", 5010);

    if(socket_fd == -1)
    {
        printf("unable to connect to tracker");
        exit(1);
    }

    char buffer[256];
    //here we have connection
    //printf("connected, pls enter a message: \n");
    bzero(buffer, sizeof(buffer));
    fgets(buffer, sizeof(buffer), stdin);   //get the msg in a buffer -> msg body

    //build msg then serialize then send to the server
    message_t *message = message_constructor_from_params('0', VIEW_FILE_LIST, '0', sizeof(buffer), (byte_t*)buffer);
    byte_t *serialized_message = serialize_message(message);
    
    printf("Sending VIEW FILE LIST request...\n");
    int sent_bytes = write(socket_fd, serialized_message, (message->header->body_size+12));
    if (sent_bytes < 0)
    {
        printf("unable to send msg to the server\n");
    }

    //also read the response from the tracker server in order to display the data on the screen
    byte_t raw_response[512];
    int read_bytes = read(socket_fd, raw_response, 512);

    if (read_bytes < 0)
    {
        printf("unable to read response!\n");
    }
    message_t *response = message_constructor_from_raw(raw_response);

    printf("Available files:\n");
    printf("              md5                    name      size\n");
    // print_message(response);
    a_files = deserialize_file_array(response->body, response->header->body_size);
    a_files_number = response->header->body_size / FILE_STRUCT_SIZE;

    for (int i = 0; i < (response->header->body_size / FILE_STRUCT_SIZE); i++)
    {
        for (int j = 0; j < HASH_SIZE; j++)
            putchar(a_files[i].hash[j]);
        putchar(' ');
        putchar(' ');
        putchar(' ');

        for(int j = 0; j < NAME_SIZE; j++)
            putchar(a_files[i].name[j]);

        putchar(' ');

        printf("    %d\n", a_files[i].size);
    }

    close(socket_fd);
}

void main_loop()
{
    printf("P2P client v0.1\n");
    printf("Usage:\n");
    printf("list - lists file available that connected peers have ready for download\n");
    printf("d - attempts to download the file with the given md5\n");
    printf("exit - quits the program\n");

    char command[100];

    while(1)
    {
        putchar('>');
        scanf("%s", command);

        if ((strcmp(command, "list")== 0))
        {
            view_file_list_routine();
        }
        else if (strcmp(command, "d") == 0)
        {
            printf("Please give md5:\n");

            unsigned char md5[HASH_SIZE];

            scanf("%s", md5);

            //printf("fn is %s\n", md5);

            int size, found = 0;
            unsigned char file_name[NAME_SIZE];
            for (int i = 0; i < a_files_number; i++)
            {
                //printf("COMPARING ");
                //printf("%s\n", a_files[i].hash);

                if (strncmp((const char *)a_files[i].hash, (const char *)md5, HASH_SIZE) == 0)
                {
                    found = 1;
                    size = a_files[i].size;
                    memcpy(file_name, a_files[i].name, strlen((const char *)a_files[i].name));
                    file_name[(strlen((const char *)a_files[i].name))] = 0;
                    //printf("FILE IN FILES: %s\n", a_files[i].name);
                    //printf("LEN IS %d\n", strlen(a_files[i].name));
                    break;
                }
            }

            if (found == 0)
            {
                printf("File not available!\n");
                continue;
            }

            //printf("FOUND FILE NAME IS %s\n", file_name);
            download_file_routine(file_name, md5, size);
        }
        else if (strcmp(command, "exit") == 0)
        {
            break;
        }
        else
        {
            printf("Invalid command!\n");
        }
    }
}

int main(int argc, char *argv[])
{
    main_loop();
    return 0;
}
