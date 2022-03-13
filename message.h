#ifndef Message_h
#define Message_h

#define HEADER_SIZE 12
#define MAX_BODY_SIZE 1024
#define MAGIC 0x4D41474943  //the header of any message shold have those initial 5 bytes to be valid
typedef unsigned char byte_t;

typedef struct Header
{
    char magic[5] ;                   // 5 bytes
    byte_t message_type;                 // specifies if the current message is a request, response, ACK or NACK
    byte_t message_subtype;              // e.g. VIEW_LIST, TRANSFER_BYTES, AI_FISIER, etc
    byte_t response_code;                // in case of a req should be ignored; else takes values such as: OK or error_code for the response of a specific reques
    unsigned int body_size;              // the size of the body of the request
                                        // e.g. FILE_NOT_AVAILABLE in case of a DOWNLOAD request
}header_t;

byte_t *serialize_header(header_t *header);
header_t *deserialize_header(byte_t *header_raw);

header_t *header_constructor1(byte_t mess_type, byte_t mess_subtype, byte_t res_code, unsigned int body_size); // use when a request is constructed, then serialized and then sent
header_t *header_constructor2(byte_t *header_raw); // use when a request is received, and then deserialized and interpreted

typedef struct Message
{
    header_t *header;
    byte_t *body;   //variable size and contents; can be deduced from the header

    // fct which based on the header's contents will interpret the body's content => by returning a fct pointer
    // void *(*get_handler_for_body)(header_t *header);
    
}message_t;

message_t *message_constructor_from_raw(byte_t *message);
message_t *message_constructor_from_params(byte_t mess_type, byte_t mess_subtype, byte_t res_code, unsigned int body_size, byte_t *body);
byte_t *serialize_message(message_t *message);

void print_message(message_t *message);

#endif