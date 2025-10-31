#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "compression.h"
#include "zlib.h"
#include "utils.h"

int zlibInflate(compression_state* cs)
{
    int ret;

    if (!cs->started)
    {
        ret = inflateInit(&cs->stream);

        if (ret == Z_OK)
            cs->started = true;
    }

    do 
    {
        ret = inflate(&cs->stream, Z_NO_FLUSH);
        //printf("inflate ret: %d\n", ret);
    } while (ret != Z_BUF_ERROR && ret != Z_STREAM_END);

    if (ret == Z_STREAM_END)
    {
        ret = inflateEnd(&cs->stream);
    }

    return ret;
}

int zlibDeflate(compression_state* cs)
{
    int ret;
    
    if (!cs->started)
    {
        ret = deflateInit(&cs->stream, Z_DEFAULT_COMPRESSION);

        if (ret == Z_OK)
            cs->started = true;
    }

    int flush = (cs->final) ? Z_FINISH : Z_SYNC_FLUSH;
 
    do 
    {
        ret = deflate(&cs->stream, flush);
        //printf("deflate ret: %d\n", ret);
    } while(ret != Z_BUF_ERROR && ret != Z_STREAM_END);

    
    if (ret == Z_STREAM_END)
    {
        ret = deflateEnd(&cs->stream);
    }

    return ret;
}

uint8_t paeth(uint16_t a, uint16_t b, uint16_t c)
{
    uint16_t p, pa, pb, pc, pr;

    p = a + b - c;
    pa = abs(p - a);
    pb = abs(p - b);
    pc = abs(p - c);

    if (pa <= pb && pa <= pc)
        pr = a;
    else if (pb <= pc)
        pr = b;
    else
        pr = c;

    return pr;
}

void unfilter(span* filtered, const png_info* info)
{
    int scanline_length = (info->width * info->bpp) + 1; 
    int total_length = scanline_length * info->height;

    filter_type ft = 0;
    
    for (int i = 0; i < total_length; ++i)
    {
        if (i % scanline_length == 0)
        {
            ft = filtered->data[i];
            continue;
        }

        uint8_t x, a, b, c;
        
        x = filtered->data[i];

        if (i % scanline_length > info->bpp)
            a = filtered->data[i - info->bpp];
        else 
            a = 0;

        if (i / scanline_length != 0)
            b = filtered->data[i - scanline_length];
        else 
            b = 0;

        if (i / scanline_length != 0 && i % scanline_length > info->bpp)
            c = filtered->data[i - scanline_length - info->bpp];
        else
            c = 0;

        uint8_t filter_byte;

        switch (ft)
        {
            case NONE:
                filter_byte = 0;
                break;
            case SUB:
                filter_byte = a;
                break;
            case UP:
                filter_byte = b;
                break;
            case AVERAGE:
                filter_byte = (a + b) / 2;
                break;
            case PAETH:
                filter_byte = paeth(a, b, c);
                break;
        }

        filtered->data[i] = (x + filter_byte) % 256;
    }
}

void filter(span* unfiltered, span* filtered, const png_info* info)
{
    int scanline_length = (info->width * info->bpp) + 1; 
    int total_length = scanline_length * info->height;

    filter_type ft = 0;
    
    for (int i = 0; i < total_length; ++i)
    {
        if (i % scanline_length == 0)
        {
            ft = unfiltered->data[i];
            filtered->data[i] = ft;
            continue;
        }

        uint8_t x, a, b, c;
        
        x = unfiltered->data[i];

        if (i % scanline_length > info->bpp)
            a = unfiltered->data[i - info->bpp];
        else 
            a = 0;

        if (i / scanline_length != 0)
            b = unfiltered->data[i - scanline_length];
        else 
            b = 0;

        if (i / scanline_length != 0 && i % scanline_length > info->bpp)
            c = unfiltered->data[i - scanline_length - info->bpp];
        else
            c = 0;

        uint8_t filter_byte;

        switch (ft)
        {
            case NONE:
                filter_byte = 0;
                break;
            case SUB:
                filter_byte = a;
                break;
            case UP:
                filter_byte = b;
                break;
            case AVERAGE:
                filter_byte = (a + b) / 2;
                break;
            case PAETH:
                filter_byte = paeth(a, b, c);
                break;
        }

        filtered->data[i] = (x - filter_byte) % 256;
    }
}
