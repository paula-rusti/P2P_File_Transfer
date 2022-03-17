#include "file_utils.h"
#include <string.h>

#define BUFF 512

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

int main(){

    int fd = open("test_file.txt", O_APPEND, S_IRWXU);
    int peers = 7;
    segment_file(fd, peers);
    return 0;
}


