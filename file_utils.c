#include "file_utils.h"

#include <string.h>

// void segment_file(int file_name, unsigned int nr_of_peers) {

//     unsigned int nr_of_bytes;
//     struct stat sb; //only for the member st_size

//     if (fstat(file_name, &sb) == -1) {
//         perror("Could not open file\n");
//         exit(EXIT_FAILURE);
//     } 

//     nr_of_bytes = (unsigned int) sb.st_size / nr_of_peers;
//     unsigned int rest = sb.st_size - (nr_of_bytes * nr_of_peers);

//     printf("File size: %lu bytes\n", sb.st_size);
//     printf("No peers: %d peers\n", nr_of_peers);
//     printf("No bytes/peer: %d peers\n", nr_of_bytes);
//     printf("No bytes rest: %d bytes\n", rest);

//     unsigned int read_bytes;
//     bool is_first_read = true;

//     int file_name_out = open("outfile.txt", O_WRONLY | O_ASYNC, S_IRWXU);
//     int curr_file_offset;

//     for(int i = 0; i < nr_of_peers; i++) {
//         if(is_first_read) {    //add rest of data to the first peer
//             int first_time_req = nr_of_bytes + rest;
//             char buf[first_time_req + 1];
//             is_first_read = false;
//             read_bytes = read(file_name, buf, first_time_req);
            
//             printf("Read %d, bytes_req(%d), bytes_read(%d), buf_size(%u)\n", i, first_time_req, read_bytes, nr_of_bytes);
            
//             lseek(file_name_out, 0, SEEK_SET);
//             //here we should call an encryption function or a hash function in order not to send clear data over the network
//             write(file_name_out, buf, first_time_req);
//             curr_file_offset = read_bytes;
//         }
//         else {
//             char buf[nr_of_bytes + 1];
//             read_bytes = read(file_name, buf, nr_of_bytes);

//             printf("Read %d, bytes_req(%d), bytes_read(%d), buf_size(%u)\n", i, nr_of_bytes, read_bytes, nr_of_bytes);
            
//             lseek(file_name_out, curr_file_offset, SEEK_SET);
//             write(file_name_out, buf, nr_of_bytes);
//             curr_file_offset += nr_of_bytes;
//         }
//     }
//     close(file_name);
//     close(file_name_out);
// }

void write_segment(struct offset* offsets, int file) {

    size_t segment_size = offsets->end - offsets->start;

    /* opening segment to corresponding files */
    int segment = open(offsets->file_name, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
    if(!segment) {
        perror("FAILED OPENING & CREATING SEGMENT FILE");
        exit(1);
    }

    char buff[BLOCK_SIZE];
    size_t read_bytes;

    /* get segment data and write it to segment file */
    if(segment_size > BLOCK_SIZE) {
        while( (read_bytes = read(file, &buff, BLOCK_SIZE)) > 0) {
            write(segment, &buff, read_bytes);
            printf("1-DATA= \n%s", buff);
            memset(&buff, 0, read_bytes);
            
        }
    }
    else{
        read_bytes = read(file, &buff, segment_size);
        write(segment, &buff, segment_size);
        printf("2-DATA= \n%s", buff);
        memset(&buff, 0, read_bytes);
    }

    /* close */
    close(segment);
}

struct offset* segment_file(int file_name, unsigned int nr_of_peers) {
    struct offset offsets[nr_of_peers];
    unsigned int nr_of_bytes;
    struct stat sb;

    if (fstat(file_name, &sb) == -1) {
        perror("Could not open file\n");
        exit(EXIT_FAILURE);
    } 

    /* total number of bytes / peer */
    nr_of_bytes = (unsigned int) sb.st_size / nr_of_peers;

    /* if there is a rest of bytes, update them here & add them to the first peer */
    unsigned int rest_of_bytes = sb.st_size - (nr_of_bytes * nr_of_peers);

    for(int i = 0; i < nr_of_peers; i++) {


        /* for each peer add an offset i to it & do conversion*/
        char filename[33];
        int name_to_convert = sb.st_dev + i;
        snprintf(filename, 33, "%d", name_to_convert);
        offsets[i].file_name = filename;

        /* the first peer that needs to be treated with the rest of bytes */
        if(i == 0) {
            offsets[i].start = 0;
            offsets[i].end = nr_of_bytes + rest_of_bytes;
        }
        else{
            offsets[i].start = offsets[i-1].end ;
            offsets[i].end = offsets[i].start + nr_of_bytes;
        }

        /* control print*/
        printf("Offset i = %d{name(%s),start(%d),end(%d)}\n", i, offsets[i].file_name, offsets[i].start, offsets[i].end);

        /* write segment to corresponding files */
        write_segment(&offsets[i], file_name);
    }   

    return offsets;
}

int main(){

    int fd = open("test_file.txt", O_APPEND, S_IRWXU);
    int peers = 7;
    segment_file(fd, peers);
    return 0;
}
