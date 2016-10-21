#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include "params.h"
#include "frame.h"
#include "rlc.h"
#include "fwht.h"

void addDeltas(int16_t* deltas, uint8_t *ref, int stride);
void decodeFrame(CFRAME* frm, uint8_t* chref, uint8_t* lref, int16_t* chromaout, int16_t* lumaout);

#endif
