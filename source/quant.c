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

#define DEADZONE_WIDTH (5)

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
								3, 3, 3, 3, 3, 6,  6,  6,
								3, 3, 3, 3, 6, 6,  6,  8,
								3, 3, 3, 6, 6, 6,  8,  8,

};


void quantize(int16_t* coeff, int stride){
								int* quant =QUANT_TABLE;
								int i,j;
								int16_t* tmp = coeff;
								for(j=0; j<8; j++){
								for(i=0; i<8; i++) {
																*tmp >>= (*quant);
																quant++;
																if(*tmp >= -DEADZONE_WIDTH && *tmp <= DEADZONE_WIDTH)
																								*tmp = 0;

																tmp ++;
								}
								tmp += stride-8;
							}
}

void deadzone_quant(int16_t* coeff, int stride){
								int* quant = QUANT_TABLE;
								int i;
								int16_t* tmp = coeff;
								for(i=0; i<8; i++) {
																*tmp >>= (*quant);
																quant++;
																	if(*tmp >= -DEADZONE_WIDTH && *tmp <= DEADZONE_WIDTH)
																								*tmp = 0;
																tmp += stride;
								}
}

void dequantizep(int16_t* coeff, int stride){
								int* quant = QUANT_TABLE;
								int i;
								int16_t* tmp = coeff;
								for(i=0; i<8; i++) {
																*tmp <<= (*quant);
																quant++;
																tmp += stride;
								}

}

void dequantize(int16_t* coeff, int stride){
								int* quant = QUANT_TABLE;
								int i;
								int16_t* tmp = coeff;
								for(i=0; i<8; i++) {
																*tmp <<= (*quant);
																quant++;
																tmp += stride;
								}

}
