#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

#include "hashMd5.h"

void calcularHashMd5(const char *caminho, unsigned char *hash) {
    FILE *arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(1);
    }

    EVP_MD_CTX *md5Context = EVP_MD_CTX_new();
    if (!md5Context) {
        perror("Erro ao criar contexto MD5");
        fclose(arquivo);
        exit(1);
    }

    if (EVP_DigestInit_ex(md5Context, EVP_md5(), NULL) != 1) {
        perror("Erro em DigestInit");
        fclose(arquivo);
        EVP_MD_CTX_free(md5Context);
        exit(1);
    }

    unsigned char buffer[4096];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), arquivo)) > 0) {
        if (EVP_DigestUpdate(md5Context, buffer, bytesRead) != 1) {
            perror("Erro em DigestUpdate");
            fclose(arquivo);
            EVP_MD_CTX_free(md5Context);
            exit(1);
        }
    }

    unsigned int length = 0;
    if (EVP_DigestFinal_ex(md5Context, hash, &length) != 1) {
        perror("Erro em DigestFinal");
        fclose(arquivo);
        EVP_MD_CTX_free(md5Context);
        exit(1);
    }

    EVP_MD_CTX_free(md5Context);
    fclose(arquivo);
}

void salvarHashMd5(const char *nomeArquivo, const unsigned char *hash) {
    FILE *arquivo = fopen(nomeArquivo, "wb");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo para salvar o hash");
        exit(1);
    }

    fwrite(hash, 1, EVP_MD_size(EVP_md5()), arquivo);
    fclose(arquivo);
}

int conferirHashMd5(const char *arquivoHash, const char *nomeArquivo) {
    unsigned char hashCalculado[EVP_MAX_MD_SIZE];
    calcularHashMd5(nomeArquivo, hashCalculado);

    unsigned char hashLido[EVP_MAX_MD_SIZE];
    FILE *arquivo = fopen(arquivoHash, "rb");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de hash");
        return 0;
    }
    fread(hashLido, 1, EVP_MD_size(EVP_md5()), arquivo);
    fclose(arquivo);

    if (memcmp(hashCalculado, hashLido, EVP_MD_size(EVP_md5())) == 0) {
        return 1;
    } else {
        return 0;
    }
}
