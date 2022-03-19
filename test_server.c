#include "server.h"
#include "handlers.h"

int main()
{
    server_t *my_server = server_constructor("127.0.0.1", 5010, 5);
    my_server->start_listening(my_server, handle_request_tracker);
}