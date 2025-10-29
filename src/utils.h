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

#define NUM_BIT_PAIRS 4

typedef uint8_t byte;

typedef enum program_mode_e
{
    EXTRACT = 101,
    HIDE = 104
} program_mode;

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

typedef struct png_info_t 
{
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t bpp;
} png_info;

void resizeSpan(span* s, size_t size);
void destroySpan(span* s);

byte getNthBitPair(byte b, uint8_t n);
byte setLSBs(byte original, byte lsbs);

#endif
