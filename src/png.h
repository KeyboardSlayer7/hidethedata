#ifndef PNG_H
#define PNG_H

#include <stdio.h>

#include "utils.h"

#define PNG_HEADER_LENGTH 8
#define CHUNK_TYPE_LENGTH 4

typedef struct important_info_t
{

} important_info;

void extractImportantInformation(span* data, uint32_t* width, uint32_t* height, uint32_t* bpp);

void H_processPNG(FILE* file, const char* data);
char* E_processPNG(FILE* file);

#endif //PNG_H
