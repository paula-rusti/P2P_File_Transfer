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
    unsigned int start;
    unsigned int end;
};

// struct offset *
void segment_file(int file_name, unsigned int nr_of_peers);

#endif