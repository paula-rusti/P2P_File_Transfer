#ifndef Utils_h
#define Utils_h

#define MAX_FILES_PER_PEER 5
#define FILE_STRUCT_SIZE 42 // in bytes
#define HASH_SIZE 32
#define NAME_SIZE 10

typedef struct file
{
    unsigned char hash[HASH_SIZE]; // md5, on 16 bytes so we need 32 chars for string hex representation
    unsigned char name[NAME_SIZE];
}file_t;

unsigned char* serialize_file_array(file_t *file_array, int nr_of_files);
file_t* deserialize_file_array(unsigned char* body, int body_size); //given the body of a message, return the array of file_t structures

#endif