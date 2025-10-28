#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "utils.h"

#define WHAT
#include "core.h"

ARRAY(uint32_t)

void resizeSpan(span* s, size_t size)
{
    if (s->data == NULL)
    {
        s->data = (byte*)malloc(size);
    }
    else
    {
        void* temp = realloc(s->data, size);

        if (temp != NULL)
        {
            s->data = (byte*)temp;
        }
    }

    s->capacity = size;
}

void destroySpan(span* s)
{
    free(s->data);
}

byte getNthBitPair(byte b, uint8_t n)
{
    byte bitmask = 3 << (n * 2);
    byte temp = b & bitmask;
    byte ret = temp >> (n * 2);

    return ret;
}

byte setLSBs(byte original, byte lsbs)
{
    byte bitmask = ~3;
    byte cleared = original & bitmask;
    byte ret = cleared | lsbs;

    return ret;
}
