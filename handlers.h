#ifndef Handlers_h
#define Handlers_h

#include "message.h"
#include "utils.h"

extern file_t TEST_FILE_LIST[2];

typedef enum msg_subtype
{
    VIEW_FILE_LIST = '1',   // received by tracker, sent by peer client
    SEND_FILE_LIST = '2',   // received by peer server, sent by tracker
    DOWNLOAD_FILE = '3',
    IS_FILE_PRESENT = '4',
    TRANSFER_BYTES = '5',
    ACK = '6',
    NACK = '7'
}MSG_SUBTYPE;

void handle_request_tracker(message_t *request, int socket_fd); // based on the header contents will call the required handlers; specific for tracker requests
void handle_request_peer(message_t *request, int socket_fd); // based on the header contents will call the required handlers; specific for peer requests

void handle_view_file_list(int socket_fd);
void handle_send_file_list(int socket_fd);

#endif