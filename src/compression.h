#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <stdbool.h>

#include "zlib.h"
#include "utils.h"

typedef struct compression_state_t
{
    z_stream stream;
    bool started;
    bool final;
} compression_state;

typedef enum filter_type_e
{
    NONE = 0,
    SUB = 1,
    UP = 2,
    AVERAGE = 3,
    PAETH = 4
} filter_type;

int zlibInflate(compression_state* cs);
int zlibDeflate(compression_state* cs);

uint8_t paeth(uint16_t a, uint16_t b, uint16_t c);
void unfilter(span* filtered, const png_info* info);
void filter(span* unfiltered, span* filtered, const png_info* info);

#endif //COMPRESSION_H
