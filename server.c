#include "server.h"

#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h> 

void segment_file(int file_name, unsigned int nr_of_peers) {

    unsigned int nr_of_bytes;
    struct stat sb; //only for the member st_size

    if (fstat(file_name, &sb) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    } 

    nr_of_bytes = (unsigned int) sb.st_size / nr_of_peers;
    unsigned int rest = sb.st_size - (nr_of_bytes * nr_of_peers);
    char all[sb.st_size+1];

    printf("File size: %lu bytes\n", sb.st_size);
    printf("No peers: %d peers\n", nr_of_peers);
    printf("No bytes/peer: %d peers\n", nr_of_bytes);
    printf("No bytes rest: %d bytes\n", rest);

    unsigned int read_bytes;
    bool is_first_read = true;

    int file_name_out = open("outfile.txt", O_WRONLY | O_ASYNC, S_IRWXU);
    int curr_file_offset;

    for(int i = 0; i < nr_of_peers; i++) {
        if(is_first_read) {    //add rest of data to the first peer
            int first_time_req = nr_of_bytes + rest;
            char buf[first_time_req + 1];
            is_first_read = false;
            read_bytes = read(file_name, buf, first_time_req);
            
            printf("Read %d, bytes_req(%d), bytes_read(%d), buf_size(%u)\n", i, first_time_req, read_bytes, nr_of_bytes);
            
            lseek(file_name_out, 0, SEEK_SET);
            //here we should call an encryption function or a hash function in order not to send clear data over the network
            write(file_name_out, buf, first_time_req);
            curr_file_offset = read_bytes;
        }
        else {
            char buf[nr_of_bytes + 1];
            read_bytes = read(file_name, buf, nr_of_bytes);

            printf("Read %d, bytes_req(%d), bytes_read(%d), buf_size(%u)\n", i, nr_of_bytes, read_bytes, nr_of_bytes);
            
            lseek(file_name_out, curr_file_offset, SEEK_SET);
            write(file_name_out, buf, nr_of_bytes);
            curr_file_offset += nr_of_bytes;
        }
    }
    close(file_name);
    close(file_name_out);
}

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