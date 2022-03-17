#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>

#include <fcntl.h>

#define BLOCK_SIZE 4096

//struct only to store the start and end offset
struct offset{
    char* file_name;
    unsigned int start;
    unsigned int end;
};

/**
 * @brief returns content of the designated peer
 * 
 * @param offsets 
 * @param file to read from
 */
void write_segment(struct offset* offsets, int file);

/**
 * @brief segments file for a number of peers
 * @param file_name 
 * @param nr_of_peers 
 * @return list of offsets assignated 
 */
struct offset* segment_file(int file_name, unsigned int nr_of_peers);

/**
 * @brief reconstructs file from a given list of offsets
 * @param offsets 
 */
void reconstruct_file(struct offset* offsets);

#endif