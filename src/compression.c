#include <stdbool.h>
#include <stdio.h>

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
        printf("inflate ret: %d\n", ret);
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

    int flush = (cs->final) ? Z_FINISH : Z_NO_FLUSH;
 
    do 
    {
        ret = deflate(&cs->stream, flush);
        printf("deflate ret: %d\n", ret);
    } while(ret != Z_BUF_ERROR && ret != Z_STREAM_END);

    
    if (ret == Z_STREAM_END)
    {
        printf("Ending deflate\n");
        ret = deflateEnd(&cs->stream);
        printf("Ended deflate\n");
    }

    return ret;
}
