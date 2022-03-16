#include "server.h"

#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h> 


int main(int argc, char **argv) {

    char* file_name = "test_file.txt";
    int file = open(file_name, O_RDONLY | O_ASYNC, S_IRWXU);
    if(!file) {
        perror("BAD FILE");
    }

    unsigned int nr_of_peers = 7;
    segment_file(file, nr_of_peers);

    close(file);
    return 0;
}