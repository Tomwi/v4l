#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>
#include <assert.h>

#include "fwht.h"
#include "nv12.h"
#include "rlc.h"
#include "encoder.h"

#define WIDTH (1280)
#define HEIGHT (720)
#define FPS (60)

#define MAX_PCHAIN (10)

#define WRITE
#if 0
uint8_t testblock[8*8] = {
    1,0,0,0,0,0,0,0, 
    0,1,0,0,0,0,0,0,
    0,0,1,0,0,0,0,0,
    0,0,0,1,0,0,0,0,
    0,0,0,0,1,0,0,0,
    0,0,0,0,0,1,0,0,
    0,0,0,0,0,0,1,0,
    0,0,0,0,0,0,0,1,
};

int16_t out[8*8];
#endif

uint8_t frame[WIDTH*HEIGHT], ref_frame[WIDTH*HEIGHT];
int16_t out[WIDTH*HEIGHT];
int16_t out_rlc[WIDTH*HEIGHT+WIDTH*HEIGHT/64]; // + headers for each block.

int16_t out_dec[WIDTH*HEIGHT];


int16_t tmp_block[8*8], ref_block[8*8];


inline void fillBlock(uint8_t* input, int16_t* dst){
    int i,j;
    for(i=0; i<8; i++){
        for(j=0; j<8; j++){
            *dst++  = *input++;
        }
        input += WIDTH-8;
    }
}

int itra_dec(int8_t* current, int8_t* reference, int16_t* deltablock){
    int16_t tmp[64];
    fillBlock(current, tmp);
    int varc = var(tmp);

               // calculate delta
    int k,l;
    int16_t* work = tmp;
    for(int k=0; k<8; k++){
        for(int l=0; l<8; l++){
            *deltablock++ = *work++ - *reference++;
        }
        reference += WIDTH-8;
    }
    deltablock-=64;
    int vard = var(deltablock);
    return varc <= vard;
}


int main(int argc, char** argv){
    FRAME frm;

    if(argc <= 1){
        printf("please provide input and output\n");
        return 0;
    }

    FILE* fp = fopen(argv[1], "rb");
#ifdef WRITE
    FILE* fp2 = fopen("test output", "wb");
#endif
    if(fp == NULL){
        printf("Failed to open %s\n", argv[1]);
        return 0;
    }
    if(!init_nv12_frame(WIDTH, HEIGHT, &frm)){
        printf("frame init error\n");
        return 0;
    }

    clock_t start = clock(), diff;
    int i,j,k;
    
    fseek(fp, (WIDTH*HEIGHT + WIDTH*HEIGHT/2)*302, SEEK_SET);

    uint8_t* reference = ref_frame;
    
    int16_t deltablock[64];
    int pchain=0;
    for(int k=0; k<FPS; k++){
        if(!read_nv12_frame(fp, &frm)){
            printf("Error during read\n");
            return 0;
        }
        uint8_t* input = frm.lum;
        int16_t* output = out;
        int16_t* rlco = out_rlc;
        int cnt = 0;
    
        int waspcoded = 0;
        for(j=0; j<HEIGHT/8; j++){
            for(i=0; i<WIDTH/8; i++){
                // intra code, first frame is always intra coded.
                int pframe;
                if((pframe=itra_dec(input, reference+(int)(input-frm.lum), deltablock)) || k==0 || pchain == MAX_PCHAIN){
                    fwht(input, output, WIDTH, WIDTH, 1);
                }
                // inter code
                else{
                    waspcoded = 1;
                    fwht(deltablock, output, 0, WIDTH, 0);
                }
              
                int ret = rlc(output, rlco, WIDTH, pframe);               
                int stat = derlc(rlc, out_dec, WIDTH);
#if 0
                if(stat & PFRAME_BIT){
                    
                }
#endif                 
                rlco += ret;
                output += 8;
                input += 8;
             }
            output += WIDTH*7;
            input += WIDTH*7;
        }       
        if(pchain == MAX_PCHAIN)
            pchain = 0;
        /* Increase pchain count */
        if(waspcoded)
            pchain++;
        printf("pchain %d\n", pchain); 
        printf("Compression ratio %lf \%\n",
               ((float)(WIDTH*HEIGHT))/(rlco - out_rlc)*100); 
#ifdef WRITE
    fwrite(out, 1, sizeof(int16_t)*WIDTH*HEIGHT, fp2);
#endif 
        uint16_t* swp = reference;
        reference = frm.lum;
 
       frm.lum = reference;
    } 
    diff = clock() - start;

    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
#if 0
    int16_t*    print = out;
    int8_t* print2 = frm.lum;

    for(j=0; j<8; j++){
        for(i=0; i<8; i++){
            printf("(%d,%d),", *print++, *print2++);
        }
        print+= WIDTH-8;
        print2 += WIDTH-8;
        printf("\n");
    }
#endif
    fclose(fp);

#ifdef WRITE
    fclose(fp2);
#endif

   return 0;   
}
