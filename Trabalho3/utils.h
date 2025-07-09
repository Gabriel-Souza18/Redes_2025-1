#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdbool.h>

bool checkSum(int esperado, int atual);

void logger(const char* msg, const char* filename);

#endif // UTILS_H