//#include "encoder.h"
#include <stdint.h>

void encode(char* in, char* out);

int var(int16_t* input){

    // max intermediate value could be:
    // 255^2 * 64 + (255*64)^2 < 2^{32}-1
    int32_t mean=0;
    int32_t ssum=0;
    int i;
    for(i=0; i<8*8; i++, input++){
        mean += *input;
        ssum += *input * *input;
    }
    // don't normalize 
    return ssum - mean*mean/(64*64);         
}
