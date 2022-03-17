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

//struct only to store the start and end offset
struct offset{
    int file_descriptor;
    unsigned int start;
    unsigned int end;
};

// struct offset *
struct offset* segment_file(int file_name, unsigned int nr_of_peers);
void reconstruct_file(struct offset* offsets);
#endif