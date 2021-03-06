/*
 * Copyright 2016 Tom aan de Wiel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include "params.h"
#include "frame.h"
#include "rlc.h"
#include "fwht.h"

typedef struct DECODER_META {
  unsigned int width, height;
  int16_t *rlc_data_chrm;
  int16_t *rlc_data_lum;
  int16_t *chrm_coeff;
  int16_t *lum_coeff;
}DECODER_META;

void addDeltas(int16_t *deltas, uint8_t *ref, int stride);
void decodeFrame(CFRAME *frm, uint8_t *chref, uint8_t *lref, int16_t *chromaout, int16_t *lumaout);
void decodeFrameStateless(DECODER_META* meta, uint8_t *chref, uint8_t *lref, int16_t *chromaout, int16_t *lumaout);
#endif
