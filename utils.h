#ifndef Utils_h
#define Utils_h

#define MAX_FILES_PER_PEER 5
#define FILE_STRUCT_SIZE 26 // in bytes

typedef struct file
{
    unsigned char hash[16];
    unsigned char name[10];
}file_t;

unsigned char* serialize_file_array(file_t *file_array, int nr_of_files);
file_t* deserialize_file_array(unsigned char* body, int body_size); //given the body of a message, return the array of file_t structures

#endif