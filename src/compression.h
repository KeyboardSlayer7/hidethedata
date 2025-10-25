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

int zlibInflate(compression_state* cs);
int zlibDeflate(compression_state* cs);

#endif //COMPRESSION_H
