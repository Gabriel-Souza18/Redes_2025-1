// http_handler.c
#include "httpHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const char* get_mime_type(const char* path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    if (strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".pdf") == 0) return "application/pdf";
    return "text/plain";
}

void serve_file(int client_socket, const char *file_path){
        FILE *file = fopen(file_path, "rb");
    if (!file) {
        char *response = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
        return;
    
    }
}


void handle_request(int client_socket){
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