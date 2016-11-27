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

#include "encoder.h"
#include "quant.h"

void fillBlock(uint8_t *input, int16_t *dst, int stride)
{
	int i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++)
			*dst++  = *input++;
		input += stride-8;
	}
}

int VARINTRA(int16_t *input)
{

	int32_t mean = 0;
	int32_t ret = 0;

	int i;
	int16_t *tmp = input;

	for (i = 0; i < 8*8; i++, tmp++)
		mean += *tmp;
	mean /= 64;
	tmp = input;
	for (i = 0; i < 8*8; i++, tmp++) {
		int meh = (*tmp - mean) < 0 ? -(*tmp-mean) : (*tmp-mean);

		ret +=	meh;//*meh;
	}
	return ret;

}


int VARINTER(int16_t *old, int16_t *new)
{

	int32_t ret = 0;
	int i;

	for (i = 0; i < 8*8; i++, old++, new++) {
		int meh = (*old - *new) < 0 ? -(*old - *new) : (*old-*new);

		ret +=	meh; //*meh;
	}
	return ret;
}



int decide_blocktype(uint8_t *current, uint8_t *reference, int16_t *deltablock,
  int stride)
{
	//return IBLOCK;
	int16_t tmp[64];
	int16_t old[64];

	fillBlock(current, tmp, stride);
	fillBlock(reference, old, stride);

	int vari = VARINTRA(tmp);
	int k, l;
	int16_t *work = tmp;

	for (int k = 0; k < 8; k++) {
		for (int l = 0; l < 8; l++) {

			*deltablock = *work - *reference;

			deltablock++;
			work++;
			reference++;
		}
		reference += stride-8;
	}
	deltablock -= 64;
	int vard = VARINTER(old, tmp);
//	printf("%d %d\n", sadi, sadd);
	return (vari <= vard ? IBLOCK : PBLOCK);
}

void encodeFrameStateless(ENCODER* enc, uint8_t *lref, uint8_t *cref, CFRAME *out)
{
	int i, j;
	int16_t deltablock[64];

	// encode chroma plane
	uint8_t *input = enc->chrm;

	int16_t *coeffs = out->chrm_coeff;
	int16_t *rlco = out->rlc_data_chrm;

	unsigned int width = enc->cur_resolution[0];
	unsigned int height = enc->cur_resolution[1];

	enc->waspcoded_chrm = 0;
	for (j = 0; j < height/8; j++) {
		for (i = 0; i < width/2/8; i++) {
			int blocktype = IBLOCK;
      if (cref != NULL)
					blocktype = decide_blocktype(input, cref+(int)(input-enc->chrm),
				      deltablock, width/2);

			if (enc->pchain_chrm == enc->max_pchain || blocktype == IBLOCK) {
				fwht(input, coeffs, width/2, width/2, 1);
				quantizeIntra(coeffs, width/2);
				blocktype = IBLOCK;

			} else{
				enc->waspcoded_chrm = 1;
				fwht16(deltablock, coeffs, 8, width/2, 0);
				quantizeInter(coeffs, width/2);
			}

			int ret = rlc(coeffs, rlco, width/2, blocktype);

			rlco += ret;
			// advance to next block in current row
			coeffs += 8;
			input += 8;
		}
		// advance to next row, since chroma is subsampled, divide by 2
		coeffs += (width/2)*7;
		input  += (width/2)*7;
	}
	// size in bytes
	out->chroma_sz = (unsigned long)rlco - (unsigned long)out->rlc_data_chrm;

	if (enc->pchain_chrm == enc->max_pchain)
		enc->pchain_chrm = 0;
	/* Increase pchain count */
	if (enc->waspcoded_chrm)
		enc->pchain_chrm++;

	/* INTER FRAME */
	input = enc->luma;
	coeffs = out->lum_coeff;
	rlco = out->rlc_data_lum;

	enc->waspcoded = 0;
	
	for (j = 0; j < height/8; j++) {
		for (i = 0; i < width/8; i++) {
			// intra code, first frame is always intra coded.
			int blocktype = IBLOCK;
      if (lref != NULL)
				blocktype = decide_blocktype(input, lref+(input-enc->luma), deltablock, width);
			if (enc->pchain_lum == enc->max_pchain || blocktype == IBLOCK) {
				fwht(input, coeffs, width, width, 1);
				quantizeIntra(coeffs, width);
				blocktype = IBLOCK;
			}
			// inter code
			else{
				enc->waspcoded = 1;
				fwht16(deltablock, coeffs, 8, width, 0);
				quantizeInter(coeffs, width);
			}
			int ret = rlc(coeffs, rlco, width, blocktype);

			rlco += ret;
			coeffs += 8;
			input += 8;
		}
		coeffs += width*7;
		input += width*7;
	}
	out->lum_sz = (unsigned long)rlco - (unsigned long)out->rlc_data_lum;
	if (enc->pchain_lum == enc->max_pchain)
		enc->pchain_lum = 0;
	/* Increase pchain count */
	if (enc->waspcoded)
		enc->pchain_lum++;
}

void encodeFrame(RAW_FRAME *frm, uint8_t *lref, uint8_t *cref, CFRAME *out, int* pcount)
{
	int i, j;

	/* TODO: make encoder state structure? */
	static int pchain_chrm;
	static int pchain_lum;

	int16_t deltablock[64];

	// encode chroma plane
	uint8_t *input = frm->chrm;

	int16_t *coeffs = out->chrm_coeff;
	int16_t *rlco = out->rlc_data_chrm;

	int waspcoded_chrm = 0;

	for (j = 0; j < frm->height/8; j++) {
		for (i = 0; i < frm->width/2/8; i++) {
			int blocktype = IBLOCK;
      if (cref != NULL)
	blocktype = decide_blocktype(input, cref+(int)(input-frm->chrm),
				      deltablock, frm->width/2);

			if (pchain_chrm == MAX_PCHAIN || blocktype == IBLOCK) {
				fwht(input, coeffs, frm->width/2, frm->width/2, 1);
				quantizeIntra(coeffs, frm->width/2);
				blocktype = IBLOCK;

			} else{
				waspcoded_chrm = 1;
				pcount[0]++;
				fwht16(deltablock, coeffs, 8, frm->width/2, 0);
				quantizeInter(coeffs, frm->width/2);
			}

			int ret = rlc(coeffs, rlco, frm->width/2, blocktype);

			rlco += ret;
			// advance to next block in current row
			coeffs += 8;
			input += 8;
		}
		// advance to next row, since chroma is subsampled, divide by 2
		coeffs += (frm->width/2)*7;
		input  += (frm->width/2)*7;
	}
	// size in bytes
	out->chroma_sz = (unsigned long)rlco - (unsigned long)out->rlc_data_chrm;

	if (pchain_chrm == MAX_PCHAIN)
		pchain_chrm = 0;
	/* Increase pchain count */
	if (waspcoded_chrm)
		pchain_chrm++;

	/* INTER FRAME */
	input = frm->lum;
	coeffs = out->lum_coeff;
	rlco = out->rlc_data_lum;

	int waspcoded = 0;

	for (j = 0; j < frm->height/8; j++) {
		for (i = 0; i < frm->width/8; i++) {
			// intra code, first frame is always intra coded.
			int blocktype = IBLOCK;
      if (lref != NULL)
	blocktype = decide_blocktype(input, lref+(input-frm->lum), deltablock, frm->width);
			if (pchain_lum == MAX_PCHAIN || blocktype == IBLOCK) {
				fwht(input, coeffs, frm->width, frm->width, 1);
				quantizeIntra(coeffs, frm->width);
				blocktype = IBLOCK;
			}
			// inter code
			else{
				pcount[1]++;
				waspcoded = 1;
				fwht16(deltablock, coeffs, 8, frm->width, 0);
				quantizeInter(coeffs, frm->width);
			}
			int ret = rlc(coeffs, rlco, frm->width, blocktype);

			rlco += ret;
			coeffs += 8;
			input += 8;
		}
		coeffs += frm->width*7;
		input += frm->width*7;
	}
	out->lum_sz = (unsigned long)rlco - (unsigned long)out->rlc_data_lum;
	if (pchain_lum == MAX_PCHAIN)
		pchain_lum = 0;
	/* Increase pchain count */
	if (waspcoded)
		pchain_lum++;
}
