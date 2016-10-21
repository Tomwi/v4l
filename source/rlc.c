#include "rlc.h"
#include "frame.h"

#define ALL_ZEROS (15)

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


int rlc(int16_t* in, int16_t* output, int stride, int blocktype){

        int i = 0;
        int ret = 0;
        int x,y;
        int16_t block[8*8];

        int16_t* wp = block;

        // read in block zig-zag-wise
        int lastzero_run = 0;
        for(y=0; y<8; y++) {
                for(x=0; x<8; x++) {
                        *wp = in[x+y*stride];
                        if(*wp==0) {

                                lastzero_run++;
                        }
                        else
                                lastzero_run = 0;
                        wp++;
                }
        }

        *output++ = (blocktype == PBLOCK ? PFRAME_BIT : 0);
        ret++;

        int to_encode = 8*8 -(lastzero_run > 14 ? lastzero_run : 0);
        while(i < to_encode) {
                int cnt = 0;
                int tmp;
                // count leading zeros
                while((tmp=block[zigzag[i]]) == 0 && cnt < 14) {
                        cnt++;
                        i++;
                        if(i == to_encode) {
                                cnt--;
                                break;
                        }
                }
                // 4 bits for run, 12 for coefficient (quantization by 4)
                *output++ = (cnt | tmp << 4);
                i++;
                ret++;
        }
        if(lastzero_run > 14) {
                *output = (ALL_ZEROS | 0);
                ret++;
        }

        return ret;
}


int derlc(int16_t** rlc_in, int16_t* dwht_out, int stride){
        // header
        int16_t* input = *rlc_in;
        int16_t ret = *input++;
        int dec_count = 0;

        int16_t block[8*8];
        int16_t* wp=block;
        int i,j;
        // Now de-compress
        while(dec_count != 8*8) {

                int length = *input & 0xF;
                int coeff  = (*input++) >> 4;
                // fill rest with zeros
                if(length == 15) {
                        for(i=0; i<(64-dec_count); i++) {
                                *wp++ = 0;
                        }
                        break;
                }
                else{

                        for(i=0; i<length; i++, dec_count++)
                                *wp++ = 0;
                        *wp++ = coeff;
                        dec_count++;
                }
        }

        int length = *input & 0xF;
        int coeff  = (*input) >> 4;
        wp = block;

        for(i=0; i<64; i++) {
                int pos = zigzag[i];
                int y = pos/8;
                int x = pos%8;
                dwht_out[x + y*stride] = *wp++;
        }
        *rlc_in = input;
        return ret;
}
