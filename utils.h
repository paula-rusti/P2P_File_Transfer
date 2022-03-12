/* server.h */
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>


//struct only to store the start and end offset
struct offset{
    unsigned int start;
    unsigned int end;
};

typedef offset_ struct *offset;

struct offset *segment_file(char *file_name, unsigned int nr_of_peers);


#endif