#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>  // Para inet_ntop
#include <unistd.h>
#include "utils/httpHandler.h"
#include "utils/serverUtils.h"

#define PORT 2021

int main() {
    int listener = create_server_socket(PORT);
    struct sockaddr_in serveraddr = configure_server_address(PORT);

    if (bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("bind");
        close(listener);
        exit(1);
    }

    if (listen(listener, 10) == -1) {
        perror("listen");
        close(listener);
        exit(1);
    }

    fd_set master, read_fds;
    FD_ZERO(&master);
    FD_SET(listener, &master);
    int fdmax = listener;

    // Armazena os endereços dos clientes conectados por socket
    struct sockaddr_in client_addrs[FD_SETSIZE];
    memset(client_addrs, 0, sizeof(client_addrs));

    while (1) {
        read_fds = master;

        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            continue;
        }

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) {
                    struct sockaddr_in clientaddr;
                    socklen_t addrlen = sizeof(clientaddr);
                    int newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master);
                        if (newfd > fdmax) fdmax = newfd;
                        client_addrs[newfd] = clientaddr; // Armazena o endereço do cliente
                    }
                } else {
                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addrs[i].sin_addr, client_ip, sizeof(client_ip));

                    handle_request(i, client_ip);  // Versão atualizada
                    FD_CLR(i, &master);
                }
            }
        }
    }

    close(listener);
    return 0;
}
