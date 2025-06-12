// server_utils.c
#include "serverUtils.h"
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int create_server_socket(int port) {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1) {
        perror("socket");
        exit(1);
    }
    return listener;
}

struct sockaddr_in configure_server_address(int port) {
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);
    memset(&(serveraddr.sin_zero), '\0', 8);
    return serveraddr;
}