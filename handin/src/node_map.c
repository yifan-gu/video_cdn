#include <stdio.h>
#include <string.h>
#include <node_map.h>

int init_nodemap(NodeMap *nmap) {
    nmap->node_num = 0;
    return 0;
}

int getNodeByName(NodeMap *nmap, char *name) {
    int i;
    for (i = 0; i < nmap->node_num; i++) {
        if(strncmp(name, nmap->nodes[i].name, MAX_NODENAME_LEN) == 0) {
            return i;
        }
    }
    return -1;
}

int createNode(NodeMap *nmap, char *name) {
    int pos;
    NetNode *nd;

    pos = (nmap->node_num ++);
    nd = & nmap->nodes[pos];

    nd->is_server = 0;
    strncpy(nd->name, name, MAX_NODENAME_LEN);
    nd->name[MAX_NODENAME_LEN] = '\0';

    nd->adj.version = -1;
    /*memset(nd->adj.node_pos, 0, sizeof(nd->adj.node_pos));*/
    nd->adj.count = 0;

    return pos;
}

int getNodeOrCreateIt(NodeMap *nmap, char *name) {
    int i;

    for (i = 0; i < nmap->node_num; i++) {
        if(strncmp(name, nmap->nodes[i].name, MAX_NODENAME_LEN) == 0) {
            return i;
        }
    }

    return createNode(nmap, name);
}

void markAsServer(NodeMap *nmap, int pos) {
    nmap->nodes[pos].is_server = 1;
}


int update_adj(NodeMap *nmap, int pos, int version, char *neighbors) {
    NetNode *nd;
    char *p;
    Adjacent *adj;
    int neighbor;

    nd = & nmap->nodes[pos];
    adj = & nd->adj;

    if(version <= adj->version){
        return 0;
    }

    adj->version = version;
    adj->count = 0;

    p = strtok( neighbors, ",");
    while(p){
        if(strlen(p))
        neighbor = getNodeOrCreateIt(nmap, p);
        adj->node_pos[
                      (adj->count ++) ] = neighbor;
        p = strtok(NULL, ",");
    }

    return 0;
}
