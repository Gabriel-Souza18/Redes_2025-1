// server_utils.c
#include "serverUtils.h"
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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

void log_request(const char* client_ip, const char* method) {
    time_t now = time(NULL);
    char* timestamp = ctime(&now);
    timestamp[strcspn(timestamp, "\n")] = 0; // remove newline

    FILE* log_file = fopen("log.txt", "a");
    if (log_file) {
        fprintf(log_file, "[%s] %s - %s\n", timestamp, client_ip, method);
        fclose(log_file);
    }
}
