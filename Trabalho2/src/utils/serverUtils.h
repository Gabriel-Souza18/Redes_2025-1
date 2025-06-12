// server_utils.h
#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <netinet/in.h>

int create_server_socket(int port);
struct sockaddr_in configure_server_address(int port);

#endif