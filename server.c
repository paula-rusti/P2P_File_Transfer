#include "server.h"

struct offset *segment_file(char *file_name, unsigned int nr_of_peers)
{
    unsigned int nr_of_bytes;
    
    struct stat sb; //only for the member st_size

    if (lstat(file_name, &sb) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }    
    nr_of_bytes = (unsigned int) sb.st_size / nr_of_peers;//get the neccesary bytes to be transferred by every peer
    
    unsigned int aux = 0;//stores the last offset 
    struct offset *off_array = malloc(sizeof(struct offset) * nr_of_peers);
    for(int index = 0; index < nr_of_peers; index++)
    {
        struct offset temp;
        if (aux == 0){
            temp.start = aux;//is valid only for the first peer
            temp.end = nr_of_bytes - 1;//because we start from 0
        }
        else{
            temp.start = aux + 1;//else is end + 1
            temp.end = temp.start + nr_of_bytes;
        }
        aux = temp.end;
        off_array[index] = temp;
    }
    return off_array;
}

int main(int argc, char **argv)
{
    return 0;
}