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


int rlc(int16_t* in, int16_t* output, int stride, int pframe){
    
    int i = 0;
    int ret = 0;
    int x,y;
    int16_t block[8*8];

	int16_t* wp = block;

	for(y=0; y<8; y++){
		for(x=0; x<8; x++){
			*wp++ = in[x+y*stride];				
		}
	}

    *output++ |= (pframe ? PFRAME_BIT : 0);

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


int derlc(uint16_t* rlc_in, int16_t* dwht_out, int stride){
    // header
    int ret = *rlc_in++;
    int dec_count = 0;
    
    int16_t block[8*8];
    int16_t* wp=block;
    int i,j;
    // Now de-compress 
    while(dec_count != 8*8){
        int length = *rlc_in & 0xF;
        // Buffer is u16 to perform a logical instead of arithmetic shift
        int coeff  = (*rlc_in++) >> 4;
        for(i=0; i<length; i++, dec_count++)
            *wp++ = coeff;    
    }
    
    for(j=0; j<8; j++){
        for(i=0; i<8; i++){
            *dwht_out++ = block[zigzag[i+j*8]]; 
        }
        if(stride > 0)
            dwht_out += stride - 8;
    } 
}
