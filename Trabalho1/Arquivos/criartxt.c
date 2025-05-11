#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FILE_SIZE 1000000000 // Tamanho do arquivo em bytes
#define FILENAME_MAX_LEN 32 // Tamanho máximo para o nome do arquivo

int main() {
    char filename[FILENAME_MAX_LEN]; // Buffer para o nome do arquivo

    // Formata o nome do arquivo usando FILE_SIZE
    snprintf(filename, FILENAME_MAX_LEN, "%db.txt", FILE_SIZE);

    FILE *file = fopen(filename, "w"); // Usa a string formatada
    if (!file) {
        perror("Erro ao criar arquivo");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < FILE_SIZE; i++) {
        fputc(32 + rand() % 95, file); // Caracteres ASCII imprimíveis (32-126)
    }

    fclose(file);
    // Atualiza a mensagem de sucesso para usar o nome e tamanho corretos
    printf("Arquivo '%s' criado com %d bytes\n", filename, FILE_SIZE);
    return 0;
}