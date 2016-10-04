#ifndef RLC_H
#define RLC_H

#include <stdint.h>

#define PFRAME_BIT (1<<0)

int rlc(int16_t* block, int16_t* output, int stride, int pframe);
int derlc(int16_t* rlc_in, int16_t* dwht_out, int stride);

#endif

