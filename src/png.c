#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "png.h"
#include "utils.h"

void createModifiedPNG(FILE* file, const char* data)
{
    FILE* out = fopen("out.png", "wb");

    if (out == NULL)
    {
        printf("[ERROR] File out.png could not be created or opened\n");
        return;
    }

    span buffer = {
        .data = NULL,
        .length = 0,
        .capacity = 0
    };

    resizeSpan(&buffer, PNG_HEADER_LENGTH);
    
    fread(buffer.data, sizeof(byte), PNG_HEADER_LENGTH, file);
    fwrite(buffer.data, sizeof(byte), PNG_HEADER_LENGTH, out);
    
    uint32_t length, crc;

    char chunk_type[CHUNK_TYPE_LENGTH + 1];
    chunk_type[CHUNK_TYPE_LENGTH] = '\0';

    do
    {
        fread(&length, sizeof(byte), sizeof(uint32_t), file);
        fread(chunk_type, sizeof(byte), CHUNK_TYPE_LENGTH, file);

        length = byteswap(length);

        if (length > buffer.capacity)
            resizeSpan(&buffer, length);

        fread(buffer.data, sizeof(byte), length, file);
        buffer.length = length;

        fread(&crc, sizeof(byte), sizeof(uint32_t), file);
        
        printf("[%s, %d]\n", chunk_type, length);

        length = byteswap(length);
        
        fwrite(&length, sizeof(byte), sizeof(uint32_t), out);
        fwrite(chunk_type, sizeof(byte), CHUNK_TYPE_LENGTH, out);
        fwrite(buffer.data, sizeof(byte), buffer.length, out);
        fwrite(&crc, sizeof(byte), sizeof(uint32_t), out);

    } while(strcmp(chunk_type, "IEND") != 0);

    fclose(out);
    destroySpan(&buffer);
}

char* extractFromPNG(FILE* file)
{
    return "";
}
