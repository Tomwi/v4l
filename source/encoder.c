//#include "encoder.h"
#include <stdint.h>

void encode(char* in, char* out);

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
