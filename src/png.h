#ifndef PNG_H
#define PNG_H

#include <stdio.h>
#include <stdint.h>

#include "utils.h"

#define PNG_HEADER_LENGTH 8
#define CHUNK_TYPE_LENGTH 4

ARRAY(uint32_t)

void extractImportantInformation(span* data, png_info* info);

void H_processPNG(FILE* file, const char* data);
char* E_processPNG(FILE* file);

void modifyIDATChunks(
    span* inflated, 
    uint32_t buffer_length, 
    uint32_t_dynamic_array* chunk_lengths, 
    png_info* info, 
    const char* string, 
    FILE* out
);

void hide(span* data, const char* string);
char* extract(span* data);

#endif //PNG_H
