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

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include "params.h"
#include "fwht.h"
#include "frame.h"
#include "rlc.h"

typedef struct ENC_FRAME {
	int16_t *luma;
	int16_t *chroma;
} ENC_FRAME;

/* Encoder state structure */
typedef struct ENCODER {
	unsigned int prev_resolution[2];
	unsigned int cur_resolution[2];
	unsigned int  pchain_chrm, pchain_lum;
	unsigned int max_pchain;
	const int *quant_intra, *quant_inter;
	int waspcoded;
	int waspcoded_chrm;
	uint8_t* chrm;
	uint8_t* luma;
} ENCODER;

/* Encoder Meta-deta structure */
typedef struct ENCODER_META {
	// inputs to the encoder: pointers to working/output buffers
	int16_t *rlc_data_chrm;
	int16_t *rlc_data_lum;
	int16_t *chrm_coeff;
	int16_t *lum_coeff;
	// sizes of luma and chroma planes (compressed) (output of the encoder)
	unsigned int chroma_sz, lum_sz;
}ENCODER_META;

int var(int16_t *input);
void fillBlock(uint8_t *input, int16_t *dst, int stride);
int itra_dec(uint8_t *current, uint8_t *reference, int16_t *deltablock, int stride);
void encodeFrame(RAW_FRAME *frm, uint8_t *lref, uint8_t *cref, CFRAME *out, int* pcount);
void encodeFrameStateless(ENCODER* enc, uint8_t *lref, uint8_t *cref, ENCODER_META* meta);
#endif
