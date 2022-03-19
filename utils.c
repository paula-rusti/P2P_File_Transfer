#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

unsigned char* serialize_file_array(file_t *file_array, int nr_of_files)
{
    unsigned char* serialized_file_array = malloc(sizeof(file_t)*nr_of_files);
    for (int i=0; i<nr_of_files; i++)
    {
        unsigned char* file_entry_serialized = malloc(sizeof(unsigned char)*(16+10));
        memcpy(file_entry_serialized, file_array[i].hash, 16);
        memcpy(file_entry_serialized+16, file_array[i].name, 10);
        memcpy(serialized_file_array+i*sizeof(file_t), file_entry_serialized, sizeof(file_t));
        free(file_entry_serialized);
    }
    return serialized_file_array;
}

file_t* deserialize_file_array(unsigned char* body, int body_size)
{
    int file_nr = body_size/(16+10);
    file_t *file_array = malloc(sizeof(file_t)*(file_nr));
    
    //printf("!!!!!!!!!!!!!!body is: %s\n", body);
    for (int i=0; i<file_nr; i++)
    {
        memcpy(file_array[i].hash, body+i*(16+10), 16);
        memcpy(file_array[i].name, body+i*(16+10)+16, 10);
        //printf("!!!!!!!!!!!!!!!!!!!! %s\n%s\n", file_array[i].hash, file_array[i].name);
    }
    return file_array;
}