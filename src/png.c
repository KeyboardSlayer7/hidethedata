#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "png.h"
#include "utils.h"
#include "zlib.h"
#include "compression.h"

//ARRAY(uint32_t)
NEW_ARRAY(uint32_t)

void extractImportantInformation(span* data, png_info* info)
{
    memcpy(&info->width, data->data, sizeof(uint32_t));
    memcpy(&info->height, data->data + sizeof(uint32_t), sizeof(uint32_t));

    info->width = byteswap(info->width);
    info->height = byteswap(info->height);

    info->bit_depth = data->data[8];
    info->color_type = data->data[9];

    int channels = 0;

    switch (info->color_type)
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

    info->bpp = (info->bit_depth * channels) / 8;
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

    png_info information;
    uint32_t deflated_size = 0;
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
            extractImportantInformation(&buffer, &information);

            printf("Properties: Width: %d, Height: %d, BPP: %d\n", 
                    information.width, information.height, information.bpp);
            
            size_t projected_size = ((information.width * information.bpp) + 1) * information.height; 
            resizeSpan(&inflated, projected_size);

            cmprsn_state.stream.next_out = inflated.data;
            cmprsn_state.stream.avail_out = inflated.capacity;
        }
        else if (!strcmp(chunk_type, "IDAT"))
        {
            cmprsn_state.stream.next_in = buffer.data;
            cmprsn_state.stream.avail_in = buffer.length; 
            
            uint32_t start = 0;

            if (cmprsn_state.started)
                start = cmprsn_state.stream.total_out;
            //printf("start: %d\n", start);

            int ret = zlibInflate(&cmprsn_state);
 
            //printf("end: %lu\n", cmprsn_state.stream.total_out);
            uint32_t inflated_size = cmprsn_state.stream.total_out - start;
            append_uint32_t(&chunk_lengths, (uint32_t*)&inflated_size);
        }
        else 
        {
            if (!strcmp(previous_chunk_type, "IDAT"))
            {
                //memset(&cmprsn_state, 0, sizeof(compression_state));
                
                unfilter(&inflated, &information);
                modifyIDATChunks(&inflated, buffer.capacity, &chunk_lengths, &information, data, out);
            }
        }

        if (strcmp(chunk_type, "IDAT") != 0)
        {
            length = byteswap(length);

            fwrite(&length, sizeof(byte), sizeof(uint32_t), out);
            fwrite(chunk_type, sizeof(byte), CHUNK_TYPE_LENGTH, out);
            fwrite(buffer.data, sizeof(byte), buffer.length, out);
            fwrite(&crc, sizeof(byte), sizeof(uint32_t), out);
        }

        memcpy(previous_chunk_type, chunk_type, CHUNK_TYPE_LENGTH);

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

void modifyIDATChunks(span* inflated, uint32_t buffer_length, uint32_t_dynamic_array* chunk_lengths, png_info* info, const char* string, FILE* out)
{
    compression_state cmprsn_state;

    span filtered = SPAN_INITIALIZER;
    span deflated = SPAN_INITIALIZER;

    resizeSpan(&filtered, inflated->capacity);
    resizeSpan(&deflated, buffer_length + 4096);
    
    hide(inflated, string);

    filter(inflated, &filtered, info);
    
    int ret;

    cmprsn_state.started = false;

    cmprsn_state.stream.zalloc = NULL;
    cmprsn_state.stream.zfree = NULL;
    cmprsn_state.stream.opaque = NULL;

    //cmprsn_state.stream.next_out = buffer.data;
    //cmprsn_state.stream.avail_out = buffer.capacity;

    cmprsn_state.stream.next_in = filtered.data;

    for (int i = 0; i < chunk_lengths->size; ++i)
    {
        uint32_t length = *get_uint32_t(chunk_lengths, i);

        //cmprsn_state.stream.next_in = inflated.data + offset;
        cmprsn_state.stream.avail_in = length;
        
        cmprsn_state.stream.next_out = deflated.data;
        cmprsn_state.stream.avail_out = deflated.capacity;

        if (i == chunk_lengths->size - 1)
            cmprsn_state.final = true;

        int s = 0;

        if (cmprsn_state.started)
            s = cmprsn_state.stream.total_out;

        zlibDeflate(&cmprsn_state);

        uint32_t new_length = cmprsn_state.stream.total_out - s;
        const char* idat_chunk = "IDAT";
        uint32_t new_crc = crc32(0L, NULL, 0);

        new_crc = crc32(new_crc, (byte*)"IDAT", CHUNK_TYPE_LENGTH);
        new_crc = crc32(new_crc, deflated.data, new_length);
        new_crc = byteswap(new_crc);

        uint32_t new_length_bs = byteswap(new_length);
        
        fwrite(&new_length_bs, sizeof(byte), sizeof(uint32_t), out);
        fwrite(idat_chunk, sizeof(byte), CHUNK_TYPE_LENGTH, out);
        fwrite(deflated.data, sizeof(byte), new_length, out);
        fwrite(&new_crc, sizeof(byte), sizeof(uint32_t), out);
    }
    
    destroySpan(&filtered);
    destroySpan(&deflated);
}

void hide(span* data, const char* string)
{
    uint32_t length = strlen(string);
    byte* length_bytes = (byte*)&length;
    printf("%d bytes to be hidden\n", length);
    printf("done\n");
    for (int b = 0; b < sizeof(uint32_t); ++b)
    {
        for (int bp = 0; bp < NUM_BIT_PAIRS; ++bp)
        {
            int index = b * NUM_BIT_PAIRS + bp + 1;
            data->data[index] = setLSBs(data->data[index], getNthBitPair(length_bytes[b], bp));
        }
    }
    printf("done\n");
    for (int b = 0; b < length; ++b)
    {
        for (int bp = 0; bp < NUM_BIT_PAIRS; ++bp)
        {
            int index = b * NUM_BIT_PAIRS + bp + (sizeof(uint32_t) * NUM_BIT_PAIRS) + 1;
            data->data[index] = setLSBs(data->data[index], getNthBitPair(string[b], bp));
        }
    }
    
    printf("done\n");
}

char* extract(span* data)
{
    return "";
}
