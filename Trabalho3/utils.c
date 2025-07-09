#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

bool checkSum(int esperado, int atual) {
    return (esperado == atual)? true : false;
}
void logger(const char* msg, const char*filename) {
    if (msg != NULL || filename != NULL) {
        FILE *file = fopen(filename, "a");
        if (file != NULL) {
            fprintf(file, "%s\n", msg);
            fclose(file);
        }
    } else {
        fprintf(stderr, "Error: NULL message passed to logger.\n");
    }
}
