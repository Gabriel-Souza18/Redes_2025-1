#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>

#define PORT 2020
#define MAX_CONNECTIONS 100
#define THREAD_POOL_SIZE 4

int queue[MAX_CONNECTIONS];
int front = 0, rear = 0;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

FILE *log_file = NULL;

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

void handle_client(int client_socket) {
    char buffer[2048];
    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }

    buffer[bytes_read] = '\0';

    pthread_mutex_lock(&log_mutex);
    fprintf(log_file, "Requisição recebida:\n%s\n", buffer);
    fflush(log_file);
    pthread_mutex_unlock(&log_mutex);

    if (strncmp(buffer, "GET /image.jpg", 14) == 0) {
        const char *image_data = "*";
        char response[1024];
        snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-Length: %lu\r\n"
            "\r\n",
            strlen(image_data));

        write(client_socket, response, strlen(response));
        write(client_socket, image_data, strlen(image_data));
    } else {
        const char *body = "Not Found";
        char response[1024];
        snprintf(response, sizeof(response),
            "HTTP/1.1 404 Not Found\r\n"
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
        handle_client(client_socket);
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

    printf("Servidor rodando na porta %d...\n", PORT);

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
