#ifndef _NODE_MAP_H
#define _NODE_MAP_H

#define MAX_NODE_NUM 1024
#define MAX_NODENAME_LEN 63

typedef struct _Adjacent {
    int node_pos[MAX_NODE_NUM];
    int count;
    int version;
}Adjacent;

typedef struct _NetNode{
    char name[MAX_NODENAME_LEN + 1];
    int is_server;
    Adjacent adj;
} NetNode;

typedef struct _Nodemap {
    int node_num;
    NetNode nodes[MAX_NODE_NUM];
} NodeMap ;


int init_nodemap(NodeMap *nmap);
int getNodeByName(NodeMap *nmap, char *name);
int createNode(NodeMap *nmap, char *name);
int getNodeOrCreateIt(NodeMap *nmap, char *name);
void markAsServer(NodeMap *nmap, int pos);

int update_adj(NodeMap *nmap, int pos, int version, char *neighbors);

const char* find_closest_server(NodeMap *, char *start);

#endif // for #ifndef _NODE_MAP_H
