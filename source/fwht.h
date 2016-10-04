#ifndef FWHT_H
#define FWHT_H

#include <stdint.h>

void fwht(uint8_t* block, int16_t* out, int istride, int ostride, int intra);
void ifwht(int16_t* block, int16_t* output_block, int istride, int ostride, int intra);
void fwht16(int16_t* block, int16_t* output_block, int istride, int ostride, int intra);
#endif
