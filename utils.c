#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

unsigned char* serialize_file_array(file_t *file_array, int nr_of_files)
{
    unsigned char* serialized_file_array = malloc(sizeof(file_t) * nr_of_files);
    for (int i = 0; i < nr_of_files; i++)
    {
        unsigned char* file_entry_serialized = malloc(sizeof(unsigned char) * (HASH_SIZE + NAME_SIZE));
        memcpy(file_entry_serialized, file_array[i].hash, HASH_SIZE);
        memcpy(file_entry_serialized + HASH_SIZE, file_array[i].name, NAME_SIZE);
        memcpy(serialized_file_array + i * sizeof(file_t), file_entry_serialized, sizeof(file_t));
        free(file_entry_serialized);
    }
    return serialized_file_array;
}

file_t* deserialize_file_array(unsigned char* body, int body_size)
{
    int file_nr = body_size / (HASH_SIZE + NAME_SIZE);
    file_t *file_array = malloc(sizeof(file_t) * (file_nr));
    
    //printf("!!!!!!!!!!!!!!body is: %s\n", body);
    for (int i = 0; i < file_nr; i++)
    {
        memcpy(file_array[i].hash, body + i * (HASH_SIZE + NAME_SIZE), HASH_SIZE);
        memcpy(file_array[i].name, body + i * (HASH_SIZE + NAME_SIZE) + HASH_SIZE, NAME_SIZE);
        //printf("!!!!!!!!!!!!!!!!!!!! %s\n%s\n", file_array[i].hash, file_array[i].name);
    }
    return file_array;
}