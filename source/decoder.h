#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include "params.h"

void addDeltas(int16_t* deltas, uint8_t *ref, uint8_t* input, int stride);

#endif
