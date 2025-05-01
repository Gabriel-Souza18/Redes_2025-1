#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

#include "hashMd5.h"

void calcularHashMd5(const char *caminho,unsigned char *hash){
    FILE *arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(1);
    }

    MD5_CTX md5Context;
    MD5_Init(&md5Context);

    unsigned char buffer[4096]; // Tamanho do buffer de leitura
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), arquivo)) > 0) {
        MD5_Update(&md5Context, buffer, bytesRead);
    }
    MD5_Final(hash, &md5Context);
    fclose(arquivo);

}
void salvarHashMd5(const char *nomeArquivo,const unsigned char *hash ){
    FILE *arquivo = fopen(nomeArquivo, "wb");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo para salvar o hash");
        exit(1);
    }

    // Escrever o hash no arquivo
    fwrite(hash, 1, MD5_DIGEST_LENGTH, arquivo);
    fclose(arquivo);
}


int conferirHashMd5(const char *arquivoHash, const char *nomeArquivo){
    //calcular o hash do arquivo
    unsigned char hashCalculado[MD5_DIGEST_LENGTH];
    calcularHashMd5(nomeArquivo, hashCalculado);

    // Ler o hash do arquivo
    unsigned char hashLido[MD5_DIGEST_LENGTH];
    FILE *arquivo = fopen(arquivoHash, "rb");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de hash");
        return 0;
    }
    fread(hashLido, 1, MD5_DIGEST_LENGTH, arquivo);
    fclose(arquivo);

    // Comparar os hashes
    printf("Hash calculado: ");
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        printf("%02x", hashCalculado[i]);
    }
    printf("\nHash lido: ");    
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        printf("%02x", hashLido[i]);
    }
    printf("\n");
    if (memcmp(hashCalculado, hashLido, MD5_DIGEST_LENGTH) == 0) {
        return 1; // Hashs iguais
    } else {
        return 0; // Hashs diferentes
    }
}