#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

unsigned char* serialize_file_array(file_t *file_array, int nr_of_files)
{   
    unsigned char* serialized_file_array = malloc(sizeof(file_t) * nr_of_files);
    for (int i = 0; i < nr_of_files; i++)
    {
        unsigned char* file_entry_serialized = malloc(sizeof(unsigned char) * (HASH_SIZE + NAME_SIZE + sizeof(int)));
        memcpy(file_entry_serialized, file_array[i].hash, HASH_SIZE);
        memcpy(file_entry_serialized + HASH_SIZE, file_array[i].name, NAME_SIZE);
        *(int *)(file_entry_serialized + HASH_SIZE + NAME_SIZE) = file_array[i].size;
        memcpy(serialized_file_array + i * FILE_STRUCT_SIZE, file_entry_serialized, FILE_STRUCT_SIZE);

        free(file_entry_serialized);
    }
    return serialized_file_array;
}

file_t* deserialize_file_array(unsigned char* body, int body_size)
{
    int file_nr = body_size / (FILE_STRUCT_SIZE);
    file_t *file_array = malloc(sizeof(file_t) * (file_nr));

    for (int i = 0; i < file_nr; i++)
    {
        memcpy(file_array[i].hash, body + i * FILE_STRUCT_SIZE, HASH_SIZE);
        memcpy(file_array[i].name, body + i * FILE_STRUCT_SIZE + HASH_SIZE, NAME_SIZE);
        //file_array[i].size = *(int *)(body + i * FILE_STRUCT_SIZE + HASH_SIZE + NAME_SIZE);
        memcpy(&file_array[i].size, (body + i * FILE_STRUCT_SIZE + HASH_SIZE + NAME_SIZE), sizeof(int));
    }
    return file_array;
}