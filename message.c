#include "message.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

header_t *header_constructor1(byte_t mess_type, byte_t mess_subtype, byte_t res_code, unsigned int body_size)
{
    header_t *header = malloc(sizeof(header_t));
    
    header -> magic[0] = 0x4d; 
    header -> magic[1] = 0x41, 
    header -> magic[2] = 0x47;
    header -> magic[3] = 0x49;
    header -> magic[4] = 0x43;
    header -> message_type = mess_type;
    header -> message_subtype = mess_subtype;
    header -> response_code = res_code;
    header -> body_size = body_size;

    return header;
}

header_t *header_constructor2(byte_t *raw_header)
{
    header_t *header;
    header = deserialize_header(raw_header);
    return header;
}

message_t *message_constructor_from_raw(byte_t *raw_message)
{
    message_t *message = malloc(sizeof(message_t));
    message->body = malloc(MAX_BODY_SIZE*sizeof(byte_t));
    byte_t raw_header[HEADER_SIZE];
    
    for(int i=0; i<HEADER_SIZE; i++)
    {
        raw_header[i] = raw_message[i];
    }
    message->header = deserialize_header(raw_header);
    
    for(int i=0; i<message->header->body_size; i++)
    {
        message->body[i] = raw_message[i+HEADER_SIZE];
    }
    
    return message;
}

message_t *message_constructor_from_params(byte_t mess_type, byte_t mess_subtype, byte_t res_code, unsigned int body_size, byte_t *body)
{
    message_t *message = malloc(sizeof(message_t));
    message->body = malloc(sizeof(byte_t)*body_size);
    header_t *header = header_constructor1(mess_type, mess_subtype, res_code, body_size);

    message->header = header;
    memcpy(message->body, body, body_size);
    return message;
}

byte_t *serialize_message(message_t *message)
{
    byte_t *message_serial = malloc(sizeof(byte_t)*(HEADER_SIZE+message->header->body_size));
    memcpy(message_serial, serialize_header(message->header), HEADER_SIZE);
    memcpy(message_serial+HEADER_SIZE, message->body, message->header->body_size);
    return message_serial;
}

byte_t *serialize_header(header_t *header)
{
    byte_t *raw_header = malloc(HEADER_SIZE*sizeof(byte_t));

    for(int i=0; i<5; i++)
    {
        raw_header[i]=header->magic[i];
    }
    raw_header[5]=header->message_type;
    raw_header[6]=header->message_subtype;
    raw_header[7]=header->response_code;
    *(int *)(raw_header+8) = header->body_size;

    return raw_header;
}

header_t *deserialize_header(byte_t *header_raw)
{
    header_t *header = malloc(sizeof(header_t));

    for (int i=0; i<5; i++)
    {
        header->magic[i]=header_raw[i];
    }
    header->message_type = header_raw[5];
    header->message_subtype = header_raw[6];
    header->response_code = header_raw[7];
    header->body_size = *(int *)(header_raw+8);

    return header;
}

void print_message(message_t* message)
{
    printf("hex: %2x char: %c\n", message->header->magic[0], message->header->magic[0]);
    printf("hex: %2x char: %c\n", message->header->magic[1], message->header->magic[1]);
    printf("hex: %2x char: %c\n", message->header->magic[2], message->header->magic[2]);
    printf("hex: %2x char: %c\n", message->header->magic[3], message->header->magic[3]);
    printf("hex: %2x char: %c\n", message->header->magic[4], message->header->magic[4]);

    printf("hex: %2x char: %c\n", message->header->message_type, message->header->message_type);
    printf("hex: %2x char: %c\n", message->header->message_subtype, message->header->message_subtype);
    printf("hex: %2x char: %c\n", message->header->response_code, message->header->response_code);
    printf("hex: %2x int: %d\n",  message->header->body_size, message->header->body_size);

    printf("body: %s\n", message->body);
}