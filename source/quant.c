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

#include "quant.h"

#define DEADZONE_WIDTH (15)

const int QUANT_TABLE[] = {
	2, 2, 2, 2, 2, 2,  2,  2,
	2, 2, 2, 2, 2, 2,  2,  2,
	2, 2, 2, 2, 2, 2,  2,  3,
	2, 2, 2, 2, 2, 2,  3,  6,
	2, 2, 2, 2, 2, 3,  6,  6,
	2, 2, 2, 2, 3, 6,  6,  6,
	2, 2, 2, 3, 6, 6,  6,  6,
	2, 2, 3, 6, 6, 6,  6,  8,
};


const int QUANT_TABLE_P[] = {
	3, 3, 3, 3, 3, 3,  3,  3,
	3, 3, 3, 3, 3, 3,  3,  3,
	3, 3, 3, 3, 3, 3,  3,  3,
	3, 3, 3, 3, 3, 3,  3,  6,
	3, 3, 3, 3, 3, 3,  6,  6,
	3, 3, 3, 3, 3, 6,  6,  9,
	3, 3, 3, 3, 6, 6,  9,  9,
	3, 3, 3, 6, 6, 9,  9,  10,
};

void quantizeIntra(int16_t *coeff, int stride)
{
	const int *quant = QUANT_TABLE;
	int i, j;
	int16_t *tmp = coeff;

	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i++) {
			*tmp >>= (*quant);
			quant++;
			if (*tmp >= -DEADZONE_WIDTH && *tmp <= DEADZONE_WIDTH)
				*tmp = 0;

			tmp++;
		}
		tmp += stride-8;
	}
}

void dequantizeIntra(int16_t *coeff, int stride)
{
	const int *quant = QUANT_TABLE;
	int i, j;
	int16_t *tmp = coeff;

	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i++) {
			*tmp <<= (*quant);
			quant++;
			tmp++;
		}
		tmp += stride-8;
	}

}

void quantizeInter(int16_t *coeff, int stride)
{
	const int *quant = QUANT_TABLE_P;
	int i, j;
	int16_t *tmp = coeff;

	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i++) {
			*tmp >>= (*quant);
			quant++;
			if (*tmp >= -DEADZONE_WIDTH && *tmp <= DEADZONE_WIDTH)
				*tmp = 0;
			tmp++;
		}
		tmp += stride-8;
	}
}

void dequantizeInter(int16_t *coeff, int stride)
{
	const int *quant = QUANT_TABLE_P;
	int i, j;
	int16_t *tmp = coeff;

	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i++) {
			*tmp <<= (*quant);
			quant++;
			tmp++;
		}
		tmp += stride-8;
	}

}
