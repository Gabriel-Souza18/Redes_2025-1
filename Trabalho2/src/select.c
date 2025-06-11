#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 2020
#define BUFFER_SIZE 4096
#define WWW_ROOT "www/"

const char* get_mime_type(const char* path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    if (strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".pdf") == 0) return "application/pdf";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    return "text/plain";
}

void serve_file(int client_socket, const char* file_path) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        char *response = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Get MIME type
    const char *mime_type = get_mime_type(file_path);

    // Send headers
    char headers[1024];
    int headers_len = snprintf(
        headers, 
        sizeof(headers),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n\r\n",
        mime_type, 
        file_size
    );
    send(client_socket, headers, headers_len, 0);

    // Send file content
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }
    fclose(file);
}

int main() {
    int listener, newfd;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    fd_set master, read_fds;
    int fdmax;

    // Criar socket
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // Configurar endereço
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);
    memset(&(serveraddr.sin_zero), '\0', 8);

    // Vincular socket
    if (bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("bind");
        close(listener);
        exit(1);
    }

    // Escutar conexões
    if (listen(listener, 10) == -1) {
        perror("listen");
        close(listener);
        exit(1);
    }

    printf("Servidor rodando na porta %d\n", PORT);
    printf("Acesse: http://localhost:%d\n", PORT);

    // Configurar conjuntos de sockets
    FD_ZERO(&master);
    FD_SET(listener, &master);
    fdmax = listener;

    while (1) {
        read_fds = master;
        
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            continue;
        }

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) {
                    // Aceitar nova conexão
                    if ((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master);
                        if (newfd > fdmax) fdmax = newfd;
                        printf("Nova conexão (socket %d)\n", newfd);
                    }
                } else {
                    char buffer[BUFFER_SIZE];
                    ssize_t bytes_received = recv(i, buffer, BUFFER_SIZE - 1, 0);
                    
                    if (bytes_received <= 0) {
                        close(i);
                        FD_CLR(i, &master);
                        printf("Conexão fechada (socket %d)\n", i);
                    } else {
                        buffer[bytes_received] = '\0';
                        
                        // Parse HTTP request (first line)
                        char method[10], path[1024];
                        sscanf(buffer, "%s %s", method, path);
                        
                        // Default to index.html for root
                        if (strcmp(path, "/") == 0) {
                            strcpy(path, "/index.html");
                        }
                        
                        // Build file path
                        char file_path[2048];
                        snprintf(file_path, sizeof(file_path), "%s%s", WWW_ROOT, path + 1);
                        
                        printf("Requisitado: %s -> %s\n", path, file_path);
                        serve_file(i, file_path);
                        
                        close(i);
                        FD_CLR(i, &master);
                        printf("Resposta enviada (socket %d)\n", i);
                    }
                }
            }
        }
    }

    close(listener);
    return 0;
}