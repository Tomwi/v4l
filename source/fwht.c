#include "fwht.h"

/*
 * 8x8 Fast Walsh Hadamard Transform in sequency order
 * based on the paper:
 * A Recursive Algorithm for Sequency-Ordered Fast Walsh
 * Transforms, R.D. Brown
 */

void fwht(uint8_t* block, int16_t* output_block){
    /* Processes rows */
    int i;

    // we'll need more than 8 bits for the transformed coefficients
    // use native unit of cpu
    int workspace1[8], workspace2[8];

    uint8_t* tmp = block;
    int16_t* out = output_block;

    for(i=0; i<8; i++, tmp += 8, out+=8){
        /* Completely unrolled */
       
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

    tmp = block;
    out = output_block;

    for(i=0; i<8; i++, tmp ++, out++){
        /* Completely unrolled */
       
        // stage 1
        workspace1[0]  = out[0] + out[1*8];
        workspace1[1]  = out[0] - out[1*8];     

        workspace1[2]  = out[2*8] + out[3*8];
        workspace1[3]  = out[2*8] - out[3*8];

        workspace1[4]  = out[4*8] + out[5*8];
        workspace1[5]  = out[4*8] - out[5*8];

        workspace1[6]  = out[6*8] + out[7*8];
        workspace1[7]  = out[6*8] - out[7*8];
        
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
        out[0*8] = (workspace2[0] + workspace2[4]) >> 2;
        out[1*8] = (workspace2[0] - workspace2[4]) >> 2;
        out[2*8] = (workspace2[1] - workspace2[5])>> 2;
        out[3*8] = (workspace2[1] + workspace2[5])>>2;
        out[4*8] = (workspace2[2] + workspace2[6])>>2;
        out[5*8] = (workspace2[2] - workspace2[6])>>2;
        out[6*8] = (workspace2[3] - workspace2[7])>>2;
        out[7*8] = (workspace2[3] + workspace2[7])>>2;
    }
}

