// http_handler.c
#include "httpHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const char* get_mime_type(const char* path) {
    // Implementação existente...
}

void serve_file(int client_socket, const char* file_path) {
    // Implementação existente...
}

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received <= 0) {
        close(client_socket);
        return;
    }
    
    buffer[bytes_received] = '\0';
    
    char method[10], path[1024];
    sscanf(buffer, "%s %s", method, path);
    
    if (strcmp(path, "/") == 0) {
        strcpy(path, "/index.html");
    }
    
    char file_path[2048];
    snprintf(file_path, sizeof(file_path), "%s%s", WWW_ROOT, path + 1);
    
    serve_file(client_socket, file_path);
    close(client_socket);
}