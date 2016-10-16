#include "decoder.h"

void addDeltas(int16_t* deltas, uint8_t *ref, uint8_t* input, int stride){
        for(int k=0; k<8; k++) {
                for(int l=0; l<8; l++) {
                        *deltas += *ref++;
                        if(*deltas < 0) {
                                *deltas = 0;
                        }
                        else if(*deltas > 255) {
                                *deltas = 255;
                        }
                        deltas++;
                        input++;
                }
                //       printf("\n");
                input += stride-8;
                ref += stride-8;
                deltas += stride-8;
        }
}
