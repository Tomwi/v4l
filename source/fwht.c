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

#include "fwht.h"
#include <stdio.h>
#include "quant.h"

#define QUANT (2)
//#define DEBUG
/* TODO: quantize seperately */
void fwht(uint8_t* block, int16_t* output_block, int istride, int ostride, int intra){
        /* Processes rows */
        int i;

        // we'll need more than 8 bits for the transformed coefficients
        int32_t workspace1[8], workspace2[8];

        uint8_t* tmp = block;
        int16_t* out = output_block;

        int add = intra ? 256 : 0;
        for(i=0; i<8; i++, tmp += istride, out+=ostride) {
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
                // TODO: remove fugly intra/inter dependency

                out[0*ostride] = (workspace2[0] + workspace2[4]); //>>(QUANT);

                out[1*ostride] = (workspace2[0] - workspace2[4]); //>>(QUANT);
                out[2*ostride] = (workspace2[1] - workspace2[5]); //>>(QUANT);
                out[3*ostride] = (workspace2[1] + workspace2[5]); //>>(QUANT);
                out[4*ostride] = (workspace2[2] + workspace2[6]); //>>(QUANT);
                out[5*ostride] = (workspace2[2] - workspace2[6]); //>>(QUANT);
                out[6*ostride] = (workspace2[3] - workspace2[7]); //>>(QUANT);
                out[7*ostride] = (workspace2[3] + workspace2[7]); //>>(QUANT);
        }
}


void ifwht(int16_t* block, int16_t* output_block, int istride, int ostride, int intra){
        /* Processes rows */
        int i;

        // we'll need more than 8 bits for the transformed coefficients
        // use native unit of cpu
        int workspace1[8], workspace2[8];

        int16_t* tmp = block;
        int16_t* out = output_block;

        for(i=0; i<8; i++, tmp += istride, out+=ostride) {
                // stage 1
#ifdef DEBUG
                int x;
                for(x=0; x<8; x++)
                        printf("%d,", tmp[x]);
                printf("\n");
#endif
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

#ifdef DEBUG
                for(x=0; x<8; x++)
                        printf("%d,", out[x]);
                printf("\n");
#endif

        }

        out = output_block;

        for(i=0; i<8; i++, out++) {
 #ifdef DEBUG
                int x;
                for(x=0; x<8; x++)
                        printf("%d,", out[i*ostride]);
                printf("\n");
#endif
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
#if 0
                out[0*ostride] = 128*intra+((workspace2[0] + workspace2[4])>>6);
                out[1*ostride] = 128*intra+((workspace2[0] - workspace2[4])>>6);
                out[2*ostride] = 128*intra+((workspace2[1] - workspace2[5])>>6);
                out[3*ostride] = 128*intra+((workspace2[1] + workspace2[5])>>6);
                out[4*ostride] = 128*intra+((workspace2[2] + workspace2[6])>>6);
                out[5*ostride] = 128*intra+((workspace2[2] - workspace2[6])>>6);
                out[6*ostride] = 128*intra+((workspace2[3] - workspace2[7])>>6);
                out[7*ostride] = 128*intra+((workspace2[3] + workspace2[7])>>6);
#endif
                if(inter) {
                        out[0*ostride] = ((workspace2[0] + workspace2[4])); //>>(6-QUANT-inter));
                        out[1*ostride] = ((workspace2[0] - workspace2[4])); //>>(6-QUANT-inter));
                        out[2*ostride] = ((workspace2[1] - workspace2[5])); //>>(6-QUANT-inter));
                        out[3*ostride] = ((workspace2[1] + workspace2[5])); //>>(6-QUANT-inter));
                        out[4*ostride] = ((workspace2[2] + workspace2[6])); //>>(6-QUANT-inter));
                        out[5*ostride] = ((workspace2[2] - workspace2[6])); //>>(6-QUANT-inter));
                        out[6*ostride] = ((workspace2[3] - workspace2[7])); //>>(6-QUANT-inter));
                        out[7*ostride] = ((workspace2[3] + workspace2[7])); //>>(6-QUANT-inter));
                        dequantizep(out, ostride);
                        int d;
                        for(d=0; d<8; d++) {
                                out[ostride*d] >>= 6;
                        }
                }
//#if 0
                else{
                        out[0*ostride] = ((workspace2[0] + workspace2[4]));
                        out[1*ostride] = ((workspace2[0] - workspace2[4]));
                        out[2*ostride] = ((workspace2[1] - workspace2[5]));
                        out[3*ostride] = ((workspace2[1] + workspace2[5]));
                        out[4*ostride] = ((workspace2[2] + workspace2[6]));
                        out[5*ostride] = ((workspace2[2] - workspace2[6]));
                        out[6*ostride] = ((workspace2[3] - workspace2[7]));
                        out[7*ostride] = ((workspace2[3] + workspace2[7]));

                        dequantize(out, ostride);
                        int d;
                        for(d=0; d<8; d++) {
                                out[ostride*d] >>= 6;
                                out[ostride*d] += 128;
                        }



                }

//#endif
#ifdef DEBUG
                for(x=0; x<8; x++)
                        printf("%d,", out[i*ostride]);
                printf("\n");
#endif


        }
}



void fwht16(int16_t* block, int16_t* output_block, int istride, int ostride, int intra){
        /* Processes rows */
        int i;

        // we'll need more than 8 bits for the transformed coefficients
        int32_t workspace1[8], workspace2[8];

        int16_t* tmp = block;
        int16_t* out = output_block;
        ;
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
                int inter = intra ? 0 : 1;
                // stage 3

                out[0*ostride] = (workspace2[0] + workspace2[4]); //>>(QUANT+inter);
                out[1*ostride] = (workspace2[0] - workspace2[4]); //>>(QUANT+inter);
                out[2*ostride] = (workspace2[1] - workspace2[5]); //>>(QUANT+inter);
                out[3*ostride] = (workspace2[1] + workspace2[5]); //>>(QUANT+inter);
                out[4*ostride] = (workspace2[2] + workspace2[6]); //>>(QUANT+inter);
                out[5*ostride] = (workspace2[2] - workspace2[6]); //>>(QUANT+inter);
                out[6*ostride] = (workspace2[3] - workspace2[7]); //>>(QUANT+inter);
                out[7*ostride] = (workspace2[3] + workspace2[7]); //>>(QUANT+inter);
                deadzone_quant(out, ostride);
        }
}
