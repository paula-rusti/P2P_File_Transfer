#include "handlers.h"
#include "message.h"
#include <stdio.h>

void handle_request_tracker(message_t *request)
{   
    printf("TRACKER HANDLER!!!!!!!!!   \n");
    
    print_message(request);
}