#ifndef PNG_H
#define PNG_H

#include <stdio.h>

#include "utils.h"

#define PNG_HEADER_LENGTH 8
#define CHUNK_TYPE_LENGTH 4

void createModifiedPNG(FILE* file, const char* data);
char* extractFromPNG(FILE* file);

#endif //PNG_H
