#include "file_utils.h"
#define BUFF 512
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


void reconstruct_file(struct offset* offsets, int nr_of_peers) {
    
    unsigned int read_bytes;
    unsigned char buffer[BUFF];
    memcpy(buffer, 0, BUFF);//setting the file to bytes of 0

    char empty_buffer[BUFF];//empty array which we want to add as padding
    memset(&empty_buffer, 0, BUFF);//setting the memory to 0

    int file_name_out;

    if ( (file_name_out = open("outfile.txt", O_WRONLY | O_ASYNC, S_IRWXU)) < 0){
        perror("OUTPUT FILE CANNOT BE CREATED\n");
		exit(-1);
    }

    for (int file_nr = 0; file_nr < nr_of_peers; file_nr++){
        int file_name_input;
        if ( (file_name_input = open(offsets[file_nr].file_descriptor, O_RDONLY)) < 0){
        perror("File cannot be open\n");
		exit(-1);
    }
        while ((read_bytes = read(file_name_input, buffer, BUFF)) > 0)
        {
            write(file_name_out, buffer, read_bytes)
            if (read_bytes != 512)//if we read the last chunck of files, than we need to complete with the padding
	    {
		    memset(&empty_buffer, 0, 512-r);
		    write(fd, empty_buffer, 512-r);
	    }
        }
    }

}