#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>

#define PORT 2020
#define MAX_CONNECTIONS 100
#define THREAD_POOL_SIZE 4

int queue[MAX_CONNECTIONS];
int front = 0, rear = 0;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *log_file = NULL;

const char* get_content_type(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    if (strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".pdf") == 0) return "application/pdf";

    return "application/octet-stream";
}

void log_request(struct sockaddr_in *client_addr, const char *request) {
    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    pthread_mutex_lock(&log_mutex);
    fprintf(log_file, "[%s] %s - %s\n",
        time_str,
        inet_ntoa(client_addr->sin_addr),
        request);
    fflush(log_file);
    pthread_mutex_unlock(&log_mutex);
}

void enqueue(int client_socket) {
    pthread_mutex_lock(&queue_mutex);
    queue[rear] = client_socket;
    rear = (rear + 1) % MAX_CONNECTIONS;
    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&queue_mutex);
}

int dequeue() {
    pthread_mutex_lock(&queue_mutex);
    while (front == rear) {
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }
    int client_socket = queue[front];
    front = (front + 1) % MAX_CONNECTIONS;
    pthread_mutex_unlock(&queue_mutex);
    return client_socket;
}

void handle_client(int client_socket, struct sockaddr_in client_addr) {
    char buffer[2048];
    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }

    buffer[bytes_read] = '\0';
    char method[8], path[256];
    sscanf(buffer, "%s %s", method, path);
    char req_line[512];
    snprintf(req_line, sizeof(req_line), "%s %s", method, path);
    log_request(&client_addr, req_line);

    if (strcmp(method, "GET") == 0) {
        char filepath[512] = "www/index.html";
        char request_path[256] = "";

        if (sscanf(buffer, "GET /%255s", request_path) == 1) {
            if (strncmp(request_path, "www/", 4) == 0) {
                memmove(request_path, request_path + 4, strlen(request_path + 4) + 1);
            }

            if (strcmp(request_path, "") == 0) {
                strcpy(filepath, "www/index.html");
            } else {
                snprintf(filepath, sizeof(filepath), "www/%s", request_path);
            }
        } else {
            strcpy(filepath, "www/index.html");
        }

        int fd = open(filepath, O_RDONLY);
        if (fd == -1) {
            const char *body = "Arquivo não encontrado";
            char response[1024];
            snprintf(response, sizeof(response),
                     "HTTP/1.1 404 Not Found\r\n"
                     "Content-Type: text/plain; charset=utf-8\r\n"
                     "Content-Length: %lu\r\n"
                     "\r\n"
                     "%s", strlen(body), body);

            write(client_socket, response, strlen(response));
        } else {
            struct stat st;
            fstat(fd, &st);
            const char* content_type = get_content_type(filepath);
            char header[512];
            snprintf(header, sizeof(header),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: %s\r\n"
                     "Content-Length: %ld\r\n"
                     "\r\n", content_type, st.st_size);
            write(client_socket, header, strlen(header));

            ssize_t n;
            while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
                write(client_socket, buffer, n);
            }
            close(fd);
        }
    } else {
        const char *body = "Método não suportado";
        char response[1024];
        snprintf(response, sizeof(response),
                 "HTTP/1.1 400 Bad Request\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %lu\r\n"
                 "\r\n"
                 "%s", strlen(body), body);
        write(client_socket, response, strlen(response));
    }

    close(client_socket);
}


void* worker_thread(void* arg) {
    while (1) {
        int client_socket = dequeue();
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        getpeername(client_socket, (struct sockaddr*)&addr, &len);

        handle_client(client_socket, addr);
    }
    return NULL;
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    log_file = fopen("server.log", "a");
    if (!log_file) {
        perror("Erro ao abrir arquivo de log");
        exit(EXIT_FAILURE);
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket falhou");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind falhou");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen falhou");
        exit(EXIT_FAILURE);
    }

    pthread_t threads[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

    printf("Servidor (threads + fila) rodando na porta %d...\n", PORT);

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept falhou");
            continue;
        }
        enqueue(client_socket);
    }

    fclose(log_file);
    close(server_fd);
    return 0;
}
