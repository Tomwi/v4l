#include "rlc.h"


uint8_t zigzag[64] = {
    0, 
    1, 8, 
    2, 9, 16, 
    3, 10,17, 24,
    4, 11, 18, 25, 32,
    5, 12, 19, 26, 33, 40,
    6, 13, 20, 27, 34, 41, 48,
    7, 14, 21, 28, 35, 42, 49, 56,
    15, 22, 29, 36, 43, 50, 57,
    23,30,37,44, 51, 58,
    31, 38, 45, 52, 59,
    39, 46, 53, 60,
    47,54,61,
    55,62,
    63,
};


int rlc(int16_t* block, int16_t* output){
    int i = 0;
    int ret = 0;
    while(i < 64){
       int cnt = 0;
       int tmp;
       // count leading zeros
       while((tmp=block[zigzag[i]]) == 0 && cnt < 15){
           cnt++;
            i++;
            if(i == 64){
                cnt--;
                break;
            }
       }
       // 4 bits for run, 12 for coefficient (quantization by 4)
//        printf("%d leading zeros for %d, @ %d\n", cnt, tmp, i);
       *output++ = cnt | tmp << 4;
       i++;
       ret++;
    }
    return ret;
}

