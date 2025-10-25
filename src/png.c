#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "png.h"
#include "utils.h"
#include "zlib.h"
#include "compression.h"

NEW_ARRAY(uint32_t)

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
    
    uint32_t_dynamic_array chunk_lengths;
    init_dynamic_array_uint32_t(&chunk_lengths, NULL);

    uint32_t width, height;
    uint32_t bpp;

    //z_stream stream;
    //bool inflate_started = false;

    compression_state cmprsn_state;
    cmprsn_state.started = false;
    cmprsn_state.final = false;

    cmprsn_state.stream.zalloc = NULL;
    cmprsn_state.stream.zfree = NULL;
    cmprsn_state.stream.opaque = NULL;

    span buffer = SPAN_INITIALIZER;
    span inflated = SPAN_INITIALIZER;
    span deflated = SPAN_INITIALIZER;

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
            
            size_t projected_size = ((width * bpp) + 1) * height; 
            resizeSpan(&inflated, projected_size);

            cmprsn_state.stream.next_out = inflated.data;
            cmprsn_state.stream.avail_out = inflated.capacity;
        }
        else if (!strcmp(chunk_type, "IDAT"))
        {
            cmprsn_state.stream.next_in = buffer.data;
            cmprsn_state.stream.avail_in = buffer.length; 
            
            uint32_t start = cmprsn_state.stream.total_out;
            printf("start: %d\n", start);

            int ret = zlibInflate(&cmprsn_state);
 
            printf("end: %lu\n", cmprsn_state.stream.total_out);
            uint32_t inflated_size = cmprsn_state.stream.total_out - start;
            append_uint32_t(&chunk_lengths, (uint32_t*)&inflated_size);
        }
        else 
        {
            if (!strcmp(previous_chunk_type, "IDAT"))
            {
                memset(&cmprsn_state.stream, 0, sizeof(z_stream));

                int ret;
                int offset = 0;

                cmprsn_state.stream.zalloc = NULL;
                cmprsn_state.stream.zfree = NULL;
                cmprsn_state.stream.opaque = NULL;

                cmprsn_state.stream.next_out = buffer.data;
                cmprsn_state.stream.avail_out = buffer.capacity;

                for (int i = 0; i < chunk_lengths.size; ++i)
                {
                    uint32_t length = *get_uint32_t(&chunk_lengths, i);

                    cmprsn_state.stream.next_in = inflated.data + offset;
                    cmprsn_state.stream.avail_in = length;

                    zlibDeflate(&cmprsn_state);

                    offset += length;
                }
            }
        }

        //if (strcmp(chunk_type, "IDAT") != 0)
        //{
            length = byteswap(length);

            fwrite(&length, sizeof(byte), sizeof(uint32_t), out);
            fwrite(chunk_type, sizeof(byte), CHUNK_TYPE_LENGTH, out);
            fwrite(buffer.data, sizeof(byte), buffer.length, out);
            fwrite(&crc, sizeof(byte), sizeof(uint32_t), out);
        //}

    } while(strcmp(chunk_type, "IEND") != 0);
    
    for (int i = 0; i < chunk_lengths.size; ++i)
    {
        uint32_t* chunk_size = get_uint32_t(&chunk_lengths, i);
        printf("%d: %d\n", i, *chunk_size);
    }

    fclose(out);
    destroySpan(&buffer);
    destroySpan(&inflated);
    free_dynamic_array_uint32_t(&chunk_lengths);
}

char* E_processPNG(FILE* file)
{
    return "";
}
