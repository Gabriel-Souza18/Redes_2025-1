#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include <sys/socket.h>

#define BUFFER_SIZE 4096
#define WWW_ROOT "www/"

const char* get_mime_type(const char* path);
void serve_file(int client_socket, const char* file_path);
void handle_request(int client_socket);

#endif