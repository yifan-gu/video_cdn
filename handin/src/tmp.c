int proxy_listen(Proxy *p){
    int optval;
    struct sockaddr_in addr;

    /* create socket */
    p->in_sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (p->in_sock < 0) {
        logger(LOG_ERROR, "socket() failed");
        return -1;
    }
    setsockopt(p->in_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    /* set up addr */
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(p->listen_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(p->in_sock, (struct sockaddr *)&addr, sizeof(addr) < 0)) {
        logger(LOG_ERROR, "bind() failed");
        perror("");
        return -1;
    }

    if (listen(p->in_sock, MAX_CONN) < 0) {
        logger(LOG_ERROR, "listen() failed");
        return -1;
    }

    return 0;
}
