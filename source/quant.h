#ifndef QUANT_H
#define QUANT_H
#include <stdint.h>

void quantize(int16_t* coeff, int stride);
void dequantize(int16_t* coeff, int stride);
void deadzone_quant(int16_t* coeff, int stride);

#endif
