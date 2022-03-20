#ifndef Network_utils_h
#define Network_utils_h

#define NODES_NR 4

typedef struct node
{
    char ip_addr[64];
    int port;
}node_t;

extern node_t NODES_ARRAY[NODES_NR];

int connect_to_node(char *ip_addr, int port);

#endif