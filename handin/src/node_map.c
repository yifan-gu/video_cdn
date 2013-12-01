#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <node_map.h>
#include <logger.h>

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

const char* find_closest_server( NodeMap *nmap, char *client){
    int start;

    start = getNodeByName(nmap, client);
    if(start == -1){
        logger(LOG_INFO, "Client (%s) not found on graph", client);
        return NULL;
    }

    int visit[MAX_NODE_NUM];
    int q[MAX_NODE_NUM];
    int qf, qe;
    int i;
    int n, neighbor;
    Adjacent *adj;

    for (i = 0; i < nmap->node_num; i++) {
        visit[i] = 0;
    }

    visit[start] = 1;
    q[0] = start;
    qf = 0;
    qe = 1;

    while(qf < qe){
        n = q[qf++];
        adj = & nmap->nodes[n].adj;
        for (i = 0; i < adj->count; i++) {
            neighbor = adj->node_pos[i];
            if( ! visit[neighbor] ){
                q[qe++] = neighbor;
                if( nmap->nodes[neighbor].is_server ){
                    return nmap->nodes[neighbor].name;
                }
                visit[neighbor] = 1;
            }
        }
    }

    logger(LOG_INFO, "Can't found server for client (%s)", client);
    return NULL;
}
