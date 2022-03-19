#include "server.h"
#include "handlers.h"
#include <string.h>

file_t TEST_FILE_LIST[4];

int main()
{
    memcpy(TEST_FILE_LIST[0].hash, "AAAAAAAAAAAAAAAA", 16);
    memcpy(TEST_FILE_LIST[0].name, "file_1aaaa", 10);
    memcpy(TEST_FILE_LIST[1].hash, "BBBBBBBBBBBBBBBB", 16);
    memcpy(TEST_FILE_LIST[1].name, "file_2bbbb", 10);
    memcpy(TEST_FILE_LIST[2].hash, "CCCCCCCCCCCCCCCC", 16);
    memcpy(TEST_FILE_LIST[2].name, "file_3cccc", 10);


    server_t *peer_server = server_constructor("127.0.0.1", 5002, 5);
    peer_server->start_listening(peer_server, handle_request_peer);
}