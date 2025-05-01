#ifndef HASHMD5_H
#define HASHMD5_H

void calcularHashMd5(const char *caminho,unsigned char *hash);
void salvarHashMd5(const char *nomeArquivo,const unsigned char *hash);
int conferirHashMd5(const char *arquivoHash, const char *nomeArquivo);

#endif // HASHMD5_H