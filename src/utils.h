#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdint.h>

#include "core.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <intrin.h>
    #define byteswap _byteswap_ulong
#elif defined(__linux__)
    #include <byteswap.h>
    #define byteswap __builtin_bswap32
#endif

typedef uint8_t byte;

typedef struct span_t
{
    byte* data;
    size_t capacity;
    size_t length;
} span;

#define SPAN_INITIALIZER    \
    {                       \
        .data = NULL,       \
        .length = 0,        \
        .capacity = 0       \
    }

void resizeSpan(span* s, size_t size);
void destroySpan(span* s);

#endif
