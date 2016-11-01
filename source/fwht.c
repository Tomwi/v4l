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
 *
 * 8x8 Fast Walsh Hadamard Transform in sequency order
 * based on the paper:
 * A Recursive Algorithm for Sequency-Ordered Fast Walsh
 * Transforms, R.D. Brown, 1977
 */

#include <stdio.h>
#include "fwht.h"
#include "quant.h"

void fwht(uint8_t *block, int16_t *output_block, int istride, int ostride, int intra)
{
	int i;
	// we'll need more than 8 bits for the transformed coefficients
	int32_t workspace1[8], workspace2[8];

	uint8_t *tmp = block;
	int16_t *out = output_block;

	int add = intra ? 256 : 0;

	for (i = 0; i < 8; i++, tmp += istride, out += ostride) {
		// stage 1
		workspace1[0]  = tmp[0] + tmp[1] - add;
		workspace1[1]  = tmp[0] - tmp[1];

		workspace1[2]  = tmp[2] + tmp[3] - add;
		workspace1[3]  = tmp[2] - tmp[3];

		workspace1[4]  = tmp[4] + tmp[5] - add;
		workspace1[5]  = tmp[4] - tmp[5];

		workspace1[6]  = tmp[6] + tmp[7] - add;
		workspace1[7]  = tmp[6] - tmp[7];

		// stage 2
		workspace2[0] = workspace1[0] + workspace1[2];
		workspace2[1] = workspace1[0] - workspace1[2];
		workspace2[2] = workspace1[1] - workspace1[3];
		workspace2[3] = workspace1[1] + workspace1[3];

		workspace2[4] = workspace1[4] + workspace1[6];
		workspace2[5] = workspace1[4] - workspace1[6];
		workspace2[6] = workspace1[5] - workspace1[7];
		workspace2[7] = workspace1[5] + workspace1[7];

		// stage 3
		out[0] = workspace2[0] + workspace2[4];
		out[1] = workspace2[0] - workspace2[4];
		out[2] = workspace2[1] - workspace2[5];
		out[3] = workspace2[1] + workspace2[5];
		out[4] = workspace2[2] + workspace2[6];
		out[5] = workspace2[2] - workspace2[6];
		out[6] = workspace2[3] - workspace2[7];
		out[7] = workspace2[3] + workspace2[7];
	}

	out = output_block;

	for (i = 0; i < 8; i++, out++) {
		// stage 1
		workspace1[0]  = out[0] + out[1*ostride];
		workspace1[1]  = out[0] - out[1*ostride];

		workspace1[2]  = out[2*ostride] + out[3*ostride];
		workspace1[3]  = out[2*ostride] - out[3*ostride];

		workspace1[4]  = out[4*ostride] + out[5*ostride];
		workspace1[5]  = out[4*ostride] - out[5*ostride];

		workspace1[6]  = out[6*ostride] + out[7*ostride];
		workspace1[7]  = out[6*ostride] - out[7*ostride];

		// stage 2
		workspace2[0] = workspace1[0] + workspace1[2];
		workspace2[1] = workspace1[0] - workspace1[2];
		workspace2[2] = workspace1[1] - workspace1[3];
		workspace2[3] = workspace1[1] + workspace1[3];

		workspace2[4] = workspace1[4] + workspace1[6];
		workspace2[5] = workspace1[4] - workspace1[6];
		workspace2[6] = workspace1[5] - workspace1[7];
		workspace2[7] = workspace1[5] + workspace1[7];
		// stage 3
		out[0*ostride] = (workspace2[0] + workspace2[4]);
		out[1*ostride] = (workspace2[0] - workspace2[4]);
		out[2*ostride] = (workspace2[1] - workspace2[5]);
		out[3*ostride] = (workspace2[1] + workspace2[5]);
		out[4*ostride] = (workspace2[2] + workspace2[6]);
		out[5*ostride] = (workspace2[2] - workspace2[6]);
		out[6*ostride] = (workspace2[3] - workspace2[7]);
		out[7*ostride] = (workspace2[3] + workspace2[7]);
    if(!intra)
      quantizeInter(out, ostride);
	}
}


void ifwht(int16_t *block, int16_t *output_block, int istride, int ostride, int intra)
{
	int i;
	// we'll need more than 8 bits for the transformed coefficients
	// use native unit of cpu
	int workspace1[8], workspace2[8];

	int16_t *tmp = block;
	int16_t *out = output_block;

	for (i = 0; i < 8; i++, tmp += istride, out += ostride) {
		// stage 1
		workspace1[0]  = tmp[0] + tmp[1];
		workspace1[1]  = tmp[0] - tmp[1];

		workspace1[2]  = tmp[2] + tmp[3];
		workspace1[3]  = tmp[2] - tmp[3];

		workspace1[4]  = tmp[4] + tmp[5];
		workspace1[5]  = tmp[4] - tmp[5];

		workspace1[6]  = tmp[6] + tmp[7];
		workspace1[7]  = tmp[6] - tmp[7];

		// stage 2
		workspace2[0] = workspace1[0] + workspace1[2];
		workspace2[1] = workspace1[0] - workspace1[2];
		workspace2[2] = workspace1[1] - workspace1[3];
		workspace2[3] = workspace1[1] + workspace1[3];

		workspace2[4] = workspace1[4] + workspace1[6];
		workspace2[5] = workspace1[4] - workspace1[6];
		workspace2[6] = workspace1[5] - workspace1[7];
		workspace2[7] = workspace1[5] + workspace1[7];

		// stage 3
		out[0] = workspace2[0] + workspace2[4];
		out[1] = workspace2[0] - workspace2[4];
		out[2] = workspace2[1] - workspace2[5];
		out[3] = workspace2[1] + workspace2[5];
		out[4] = workspace2[2] + workspace2[6];
		out[5] = workspace2[2] - workspace2[6];
		out[6] = workspace2[3] - workspace2[7];
		out[7] = workspace2[3] + workspace2[7];
	}

	out = output_block;

	for (i = 0; i < 8; i++, out++) {
		// stage 1
		workspace1[0]  = out[0] + out[1*ostride];
		workspace1[1]  = out[0] - out[1*ostride];

		workspace1[2]  = out[2*ostride] + out[3*ostride];
		workspace1[3]  = out[2*ostride] - out[3*ostride];

		workspace1[4]  = out[4*ostride] + out[5*ostride];
		workspace1[5]  = out[4*ostride] - out[5*ostride];

		workspace1[6]  = out[6*ostride] + out[7*ostride];
		workspace1[7]  = out[6*ostride] - out[7*ostride];

		// stage 2
		workspace2[0] = workspace1[0] + workspace1[2];
		workspace2[1] = workspace1[0] - workspace1[2];
		workspace2[2] = workspace1[1] - workspace1[3];
		workspace2[3] = workspace1[1] + workspace1[3];

		workspace2[4] = workspace1[4] + workspace1[6];
		workspace2[5] = workspace1[4] - workspace1[6];
		workspace2[6] = workspace1[5] - workspace1[7];
		workspace2[7] = workspace1[5] + workspace1[7];
		int inter = intra ? 0 : 1;
		// stage 3
		if (inter) {
			out[0*ostride] = ((workspace2[0] + workspace2[4]));
			out[1*ostride] = ((workspace2[0] - workspace2[4]));
			out[2*ostride] = ((workspace2[1] - workspace2[5]));
			out[3*ostride] = ((workspace2[1] + workspace2[5]));
			out[4*ostride] = ((workspace2[2] + workspace2[6]));
			out[5*ostride] = ((workspace2[2] - workspace2[6]));
			out[6*ostride] = ((workspace2[3] - workspace2[7]));
			out[7*ostride] = ((workspace2[3] + workspace2[7]));			
			int d;

			for (d = 0; d < 8; d++) {
				out[ostride*d] >>= 6;
			}
		} else{
			out[0*ostride] = ((workspace2[0] + workspace2[4]));
			out[1*ostride] = ((workspace2[0] - workspace2[4]));
			out[2*ostride] = ((workspace2[1] - workspace2[5]));
			out[3*ostride] = ((workspace2[1] + workspace2[5]));
			out[4*ostride] = ((workspace2[2] + workspace2[6]));
			out[5*ostride] = ((workspace2[2] - workspace2[6]));
			out[6*ostride] = ((workspace2[3] - workspace2[7]));
			out[7*ostride] = ((workspace2[3] + workspace2[7]));

			//dequantizeIntra(out, ostride);
			int d;

			for (d = 0; d < 8; d++) {
				out[ostride*d] >>= 6;
				out[ostride*d] += 128;
			}
		}
	}
}

/*
 * Not the nicest way of doing it, but P-blocks get twice the range of
 * that of the I-blocks. Therefore we need a type bigger than 8 bits.
 * Furthermore values can be negative... This is just a version that
 * works with 16 signed data
 */
void fwht16(int16_t* block, int16_t* output_block, int istride, int ostride, int intra){

        int i;
        // we'll need more than 8 bits for the transformed coefficients
        int32_t workspace1[8], workspace2[8];

        int16_t* tmp = block;
        int16_t* out = output_block;

        for(i=0; i<8; i++, tmp += istride, out+=ostride) {
                // stage 1
                workspace1[0]  = tmp[0] + tmp[1];
                workspace1[1]  = tmp[0] - tmp[1];

                workspace1[2]  = tmp[2] + tmp[3];
                workspace1[3]  = tmp[2] - tmp[3];

                workspace1[4]  = tmp[4] + tmp[5];
                workspace1[5]  = tmp[4] - tmp[5];

                workspace1[6]  = tmp[6] + tmp[7];
                workspace1[7]  = tmp[6] - tmp[7];

                // stage 2
                workspace2[0] = workspace1[0] + workspace1[2];
                workspace2[1] = workspace1[0] - workspace1[2];
                workspace2[2] = workspace1[1] - workspace1[3];
                workspace2[3] = workspace1[1] + workspace1[3];

                workspace2[4] = workspace1[4] + workspace1[6];
                workspace2[5] = workspace1[4] - workspace1[6];
                workspace2[6] = workspace1[5] - workspace1[7];
                workspace2[7] = workspace1[5] + workspace1[7];

                // stage 3
                out[0] = workspace2[0] + workspace2[4];
                out[1] = workspace2[0] - workspace2[4];
                out[2] = workspace2[1] - workspace2[5];
                out[3] = workspace2[1] + workspace2[5];
                out[4] = workspace2[2] + workspace2[6];
                out[5] = workspace2[2] - workspace2[6];
                out[6] = workspace2[3] - workspace2[7];
                out[7] = workspace2[3] + workspace2[7];
        }

        out = output_block;

        for(i=0; i<8; i++, out++) {
                // stage 1
                workspace1[0]  = out[0] + out[1*ostride];
                workspace1[1]  = out[0] - out[1*ostride];

                workspace1[2]  = out[2*ostride] + out[3*ostride];
                workspace1[3]  = out[2*ostride] - out[3*ostride];

                workspace1[4]  = out[4*ostride] + out[5*ostride];
                workspace1[5]  = out[4*ostride] - out[5*ostride];

                workspace1[6]  = out[6*ostride] + out[7*ostride];
                workspace1[7]  = out[6*ostride] - out[7*ostride];

                // stage 2
                workspace2[0] = workspace1[0] + workspace1[2];
                workspace2[1] = workspace1[0] - workspace1[2];
                workspace2[2] = workspace1[1] - workspace1[3];
                workspace2[3] = workspace1[1] + workspace1[3];

                workspace2[4] = workspace1[4] + workspace1[6];
                workspace2[5] = workspace1[4] - workspace1[6];
                workspace2[6] = workspace1[5] - workspace1[7];
                workspace2[7] = workspace1[5] + workspace1[7];

                // stage 3
                out[0*ostride] = (workspace2[0] + workspace2[4]);
                out[1*ostride] = (workspace2[0] - workspace2[4]);
                out[2*ostride] = (workspace2[1] - workspace2[5]);
                out[3*ostride] = (workspace2[1] + workspace2[5]);
                out[4*ostride] = (workspace2[2] + workspace2[6]);
                out[5*ostride] = (workspace2[2] - workspace2[6]);
                out[6*ostride] = (workspace2[3] - workspace2[7]);
                out[7*ostride] = (workspace2[3] + workspace2[7]);
        }
}
