#ifndef Handlers_h
#define Handlers_h

#include "message.h"
#include "utils.h"
#include "file_utils.h"

extern file_t TEST_FILE_LIST[4];

typedef enum msg_type
{
    REQUEST = '0',
    RESPONSE = '1'
}MSG_TYPE;

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

typedef enum response_code  //only valid for messages with message type RESPONSE
{
    SUCCESS = '0',
    ERROR = '1',
    FILE_PRESENT = '2',  //raspuns afirmativ la ai fisieru
    FILE_NOT_PRESENT = '3'  //raspuns negativ la ai fisieru
}RESPONSE_CODE;

void handle_request_tracker(message_t *request, int socket_fd); // based on the header contents will call the required handlers; specific for tracker requests
void handle_request_peer(message_t *request, int socket_fd); // based on the header contents will call the required handlers; specific for peer requests

void handle_view_file_list(int socket_fd);
void handle_send_file_list(int socket_fd);
void handle_download_file(int socket_fd, byte_t md5[16]);
void handle_is_file_present(int socket_fd, byte_t md5[16]);   //received by peer servers
void handle_transfer_bytes(int socket_fd, struct offset off);


#endif