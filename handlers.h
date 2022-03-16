#ifndef Handlers_h
#define Handlers_h

#include "message.h"

void handle_request_tracker(message_t *request); // based on the header contents will call the required handlers; specific for tracker requests
void handle_request_peer(message_t *request); // based on the header contents will call the required handlers; specific for peer requests


#endif