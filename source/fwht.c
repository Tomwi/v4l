#include "fwht.h"
#include <stdio.h>
/*
 * 8x8 Fast Walsh Hadamard Transform in sequency order
 * based on the paper:
 * A Recursive Algorithm for Sequency-Ordered Fast Walsh
 * Transforms, R.D. Brown
 */
#define QUANT (2)

void fwht(uint8_t* block, int16_t* output_block, int istride, int ostride, int intra){
    /* Processes rows */
    int i;

    // we'll need more than 8 bits for the transformed coefficients
    // use native unit of cpu
    int workspace1[8], workspace2[8];

    uint8_t* tmp = block;
    int16_t* out = output_block;

    for(i=0; i<8; i++, tmp += istride, out+=ostride){
        // stage 1
        workspace1[0]  = tmp[0] + tmp[1] - 256;
        workspace1[1]  = tmp[0] - tmp[1];     

        workspace1[2]  = tmp[2] + tmp[3] - 256;
        workspace1[3]  = tmp[2] - tmp[3];

        workspace1[4]  = tmp[4] + tmp[5] - 256;
        workspace1[5]  = tmp[4] - tmp[5];

        workspace1[6]  = tmp[6] + tmp[7] - 256;
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

    for(i=0; i<8; i++, out++){
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
        out[0*ostride] = (workspace2[0] + workspace2[4])>>(QUANT+inter);
        out[1*ostride] = (workspace2[0] - workspace2[4])>>(QUANT+inter);
        out[2*ostride] = (workspace2[1] - workspace2[5])>>(QUANT+inter);
        out[3*ostride] = (workspace2[1] + workspace2[5])>>(QUANT+inter);
        out[4*ostride] = (workspace2[2] + workspace2[6])>>(QUANT+inter);
        out[5*ostride] = (workspace2[2] - workspace2[6])>>(QUANT+inter);
        out[6*ostride] = (workspace2[3] - workspace2[7])>>(QUANT+inter);
        out[7*ostride] = (workspace2[3] + workspace2[7])>>(QUANT+inter);
    }
}

