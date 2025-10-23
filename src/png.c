#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "png.h"
#include "utils.h"
#include "zlib.h"

void extractImportantInformation(span* data, uint32_t* width, uint32_t* height, uint32_t* bpp)
{
    memcpy(width, data->data, sizeof(uint32_t));
    memcpy(height, data->data + sizeof(uint32_t), sizeof(uint32_t));

    *width = byteswap(*width);
    *height = byteswap(*height);

    uint8_t bit_depth = data->data[8];
    uint8_t color_type = data->data[9];

    int channels = 0;

    switch (color_type)
    {
        case 0:
            channels = 1;
            break;
        case 2:
            channels = 3;
            break;
        case 3:
            channels = 1;
            break;
        case 4:
            channels = 2;
            break;
        case 6:
            channels = 4;
            break;
        default:
            break;
    }

    *bpp = (bit_depth * channels) / 8;
}

void H_processPNG(FILE* file, const char* data)
{
    FILE* out = fopen("out.png", "wb");

    if (out == NULL)
    {
        printf("[ERROR] File out.png could not be created or opened\n");
        return;
    }
    
    uint32_t width, height;
    uint32_t bpp;

    z_stream stream;

    stream.zalloc = NULL;
    stream.zfree = NULL;
    stream.opaque = NULL;

    span buffer = {
        .data = NULL,
        .length = 0,
        .capacity = 0
    };

    span inflated = {
        .data = NULL,
        .length = 0,
        .capacity = 0
    };

    span deflated = {
        .data = NULL,
        .length = 0,
        .capacity = 0
    };

    resizeSpan(&buffer, PNG_HEADER_LENGTH);
    
    fread(buffer.data, sizeof(byte), PNG_HEADER_LENGTH, file);
    fwrite(buffer.data, sizeof(byte), PNG_HEADER_LENGTH, out);
    
    uint32_t length, crc;
    
    char previous_chunk_type[CHUNK_TYPE_LENGTH + 1];
    previous_chunk_type[CHUNK_TYPE_LENGTH] = '\0';

    char chunk_type[CHUNK_TYPE_LENGTH + 1];
    chunk_type[CHUNK_TYPE_LENGTH] = '\0';

    do
    {
        fread(&length, sizeof(byte), sizeof(uint32_t), file);
        fread(chunk_type, sizeof(byte), CHUNK_TYPE_LENGTH, file);

        if (!strcmp(chunk_type, "IDAT"))
        {
            stream.next_in = buffer.data;
            stream.avail_in = buffer.length;

            if (strcmp(chunk_type, previous_chunk_type) != 0)
            {
                resizeSpan(&inflated, ((width * bpp) + 1) * height);

                stream.next_out = inflated.data;
                stream.avail_out = inflated.capacity;

                int ret = inflateInit(&stream);
            }
            else
            {
                int ret;

                do 
                {
                    ret = inflate(&stream, Z_NO_FLUSH);
                    //printf("inflate Z_BUF_ERROR: %d\n", ret);
                } while (ret != Z_BUF_ERROR);
            }
        }
        else
        {
            if (!strcmp(previous_chunk_type, "IDAT"))
            {
                stream.next_in = buffer.data;
                stream.avail_in = buffer.length;

                int ret;

                do 
                {
                    ret = inflate(&stream, Z_NO_FLUSH);
                    //printf("inflate Z_STREAM_ENDing: %d\n", ret);
                } while (ret != Z_STREAM_END);

                ret = inflateEnd(&stream);
            }
        }

        length = byteswap(length);

        if (length > buffer.capacity)
            resizeSpan(&buffer, length);

        fread(buffer.data, sizeof(byte), length, file);
        buffer.length = length;

        fread(&crc, sizeof(byte), sizeof(uint32_t), file);

        printf("[%s, %d]\n", chunk_type, length);
        
        if (!strcmp(chunk_type, "IHDR"))
        {
            extractImportantInformation(&buffer, &width, &height, &bpp);
            printf("Properties: Width: %d, Height: %d, BPP: %d\n", width, height, bpp);
        }

        //if (strcmp(chunk_type, "IDAT") != 0)
        //{
            length = byteswap(length);

            fwrite(&length, sizeof(byte), sizeof(uint32_t), out);
            fwrite(chunk_type, sizeof(byte), CHUNK_TYPE_LENGTH, out);
            fwrite(buffer.data, sizeof(byte), buffer.length, out);
            fwrite(&crc, sizeof(byte), sizeof(uint32_t), out);
        //}

        memcpy(previous_chunk_type, chunk_type, CHUNK_TYPE_LENGTH);

    } while(strcmp(chunk_type, "IEND") != 0);

    fclose(out);
    destroySpan(&buffer);
    destroySpan(&inflated);
}

char* E_processPNG(FILE* file)
{
    return "";
}
