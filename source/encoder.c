#include "encoder.h"

void fillBlock(uint8_t* input, int16_t* dst, int stride){
        int i,j;
        for(i=0; i<8; i++) {
                for(j=0; j<8; j++) {
                        *dst++  = *input++;
                }
                input += stride-8;
        }
}

/* Computes the sample variance */
int var(int16_t* input){

        int32_t mean=0;
        int32_t ssum=0;
        int i;
        for(i=0; i<8*8; i++, input++) {
                mean += *input;
                ssum += *input * *input;
        }
        return ssum - (mean*mean)/(64);
}

int itra_dec(uint8_t* current, uint8_t* reference, int16_t* deltablock, int stride){
        // return 1;
        int16_t tmp[64];
        fillBlock(current, tmp,stride);
        int varc = var(tmp);
        int k,l;
        int16_t* work = tmp;
        for(int k=0; k<8; k++) {
                for(int l=0; l<8; l++) {

                        *deltablock = *work - *reference;

                        deltablock++;
                        work++;
                        reference++;
                }
                reference += stride-8;
        }
        deltablock-=64;
        int vard = var(deltablock);
        return (varc <= vard ? 1 : 0);
}
