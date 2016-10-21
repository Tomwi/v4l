#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include "params.h"
#include "fwht.h"
#include "frame.h"
#include "rlc.h"

typedef struct ENC_FRAME {
        int16_t* luma;
        int16_t* chroma;
}ENC_FRAME;

int var(int16_t* input);
void fillBlock(uint8_t* input, int16_t* dst, int stride);
int itra_dec(uint8_t* current, uint8_t* reference, int16_t* deltablock, int stride);
void encodeFrame(RAW_FRAME* frm, uint8_t* lref, uint8_t* cref, CFRAME* out);

#endif
