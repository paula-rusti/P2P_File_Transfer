#include "file_utils.h"
#include <string.h>

int remove_files(struct offset* offsets, int nr_of_peers) {
    for(int i = 0; i < nr_of_peers; i++) {
       
        /* get the name of each files from corresponding peer and delete it */
        if (remove(offsets[i].file_name) != 0) {
            printf("Could not remove file: %s", offsets[i].file_name);
            return 1;
        }
    }
    return 0;
}

// void write_segment(struct offset* offsets, int file) {

//     size_t segment_size = offsets->end - offsets->start;

//     /* opening segment to corresponding files */
//     int segment = open(offsets->file_name, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
//     if(!segment) {
//         perror("FAILED OPENING & CREATING SEGMENT FILE");
//         exit(1);
//     }

//     char buff[BLOCK_SIZE];
//     size_t read_bytes;

//     /* get segment data and write it to segment file */
//     if(segment_size > BLOCK_SIZE) {
//         while( (read_bytes = read(file, &buff, BLOCK_SIZE)) > 0) {
//             write(segment, &buff, read_bytes);
//             // printf("1-DATA= \n%s", buff);
//             memset(&buff, 0, read_bytes);
            
//         }
//     }
//     else{
//         read_bytes = read(file, &buff, segment_size);
//         write(segment, &buff, segment_size);
//         // printf("2-DATA= \n%s", buff);
//         memset(&buff, 0, read_bytes);
//     }

//     /* close */
//     close(segment);
// }

// struct offset* segment_file(int file_name, unsigned int nr_of_peers) {
//     struct offset *offsets = malloc(sizeof(struct offset) * nr_of_peers);
//     unsigned int nr_of_bytes;
//     struct stat sb;

//     if (fstat(file_name, &sb) == -1) {
//         perror("Could not open file\n");
//         exit(EXIT_FAILURE);
//     } 

//     /* total number of bytes / peer */
//     nr_of_bytes = (unsigned int) sb.st_size / nr_of_peers;

//     /* if there is a rest of bytes, update them here & add them to the first peer */
//     unsigned int rest_of_bytes = sb.st_size - (nr_of_bytes * nr_of_peers);

//     for(int i = 0; i < nr_of_peers; i++) {


//         /* for each peer add an offset i to it & do conversion*/
//         char *filename = malloc(sizeof(char) * 33);
//         int name_to_convert = sb.st_dev + i;
//         snprintf(filename, 33, "%d", name_to_convert);
//         offsets[i].file_name = filename;

//         /* the first peer that needs to be treated with the rest of bytes */
//         if(i == 0) {
//             offsets[i].start = 0;
//             offsets[i].end = nr_of_bytes + rest_of_bytes;
//         }
//         else{
//             offsets[i].start = offsets[i-1].end ;
//             offsets[i].end = offsets[i].start + nr_of_bytes;
//         }

//         /* control print*/
//         //printf("Offset i = %d{name(%s),start(%d),end(%d)}\n", i, offsets[i].file_name, offsets[i].start, offsets[i].end);

//         /* write segment to corresponding files */
//         write_segment(&offsets[i], file_name);
//     }   

//     return offsets;
// }

void write_segment_socket(struct offset* offsets, int socket_fd) {

    size_t segment_size = offsets->end - offsets->start;

    /* opening segment to corresponding files */
    int file_fd = open(offsets->file_name, O_RDONLY);

    if(!file_fd) {
        perror("FAILED OPENING FILE");
        exit(1);
    }

    FILE *fp = fdopen(file_fd, "rb");

    fseek(fp, offsets->start, SEEK_SET);

    char buff[BLOCK_SIZE];
    size_t read_bytes;

    /* get segment data and write it to segment file */
    if(segment_size > BLOCK_SIZE) {
        //printf("HHHHHHHHHEEEEEEEEEEEERRRRRRRRRREEEEEEEEE\n");
        while( (read_bytes = fread( buff, 1, BLOCK_SIZE, fp)) > 0) {
            write(socket_fd, buff, read_bytes);
            // printf("1-DATA= \n%s", buff);
            memset(buff, 0, read_bytes);
            
        }
    }
    else{
        read_bytes = fread(buff, 1, BLOCK_SIZE, fp);
        //printf("READ BYTES: %ld\n", read_bytes);
        write(socket_fd, buff, segment_size);
        // printf("2-DATA= \n%s", buff);
        memset(buff, 0, read_bytes);
    }

    /* close */
    close(file_fd);
}


struct offset* segment_file_size(unsigned int file_size, unsigned int nr_of_peers) {
    struct offset *offsets = malloc(sizeof(struct offset) * nr_of_peers);
    unsigned int nr_of_bytes;
    
    printf("file_size: %d\nnr_of_peers: %d\n", file_size, nr_of_peers);
    /* total number of bytes / peer */
    nr_of_bytes =  file_size / nr_of_peers;

    /* if there is a rest of bytes, update them here & add them to the first peer */
    unsigned int rest_of_bytes = file_size - (nr_of_bytes * nr_of_peers);

    

    for(int i = 0; i < nr_of_peers; i++) {

        //printf("SEGMENT LOOP %d\n", i);
        /* for each peer add an offset i to it & do conversion*/

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
        //printf("Offset i = %d{name(%s),start(%d),end(%d)}\n", i, offsets[i].file_name, offsets[i].start, offsets[i].end);

    }   

    return offsets;
}


void reconstruct_file(struct offset *offsets, int nr_of_peers) 
{
    int out_file_fd;

    if ( (out_file_fd = open("outfile", O_WRONLY | O_CREAT, S_IRWXU)) < 0){
        perror("OUTPUT FILE CANNOT BE CREATED\n");
		exit(-1);
    }

    for (int i = 0; i < nr_of_peers; i++)
    {   
        char tmp_file_name[100];
        snprintf(tmp_file_name, 100, "%d_temp", offsets[i].start);

        int tmp_file_fd = open(tmp_file_name, O_RDONLY);
        if (tmp_file_fd == -1)
        {
            printf("Error opening temp file\n");
            exit(1);
        }

        char buff[512];

        unsigned read_bytes = 0;
        while ((read_bytes = read(tmp_file_fd, buff, 512)) > 0)
        {
            write(out_file_fd, buff, read_bytes);
        }

        close(tmp_file_fd);
    }
    
    close(out_file_fd);
}

// int main(){

//     // int fd = open("blabla", O_APPEND|O_CREAT, S_IRWXU);//in future we should change the name of the input file
//     int peers = 1;
//     struct offset* offsets = segment_file_size(23, peers);

//     for(int i = 0; i < 1; i++)
//         printf("start: %d; end: %d\n", offsets[i].start, offsets[i].end);
//     // reconstruct_file(offsets, peers);
//     // remove_files(offsets, peers);

//     return 0;
// }


