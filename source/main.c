#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fwht.h"
#include "nv12.h"

#define WIDTH (1280)
#define HEIGHT (720)
#define FPS (30)

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

uint8_t frame[WIDTH*HEIGHT];
int16_t out[WIDTH*HEIGHT];

int main(int argc, char** argv){
    FRAME frm;

    if(argc <= 1){
        printf("please provide input and output\n");
        return 0;
    }

    FILE* fp = fopen(argv[1], "rb");
    if(fp == NULL){
        printf("Failed to open %s\n", argv[1]);
        return 0;
    }
    if(!init_nv12_frame(WIDTH, HEIGHT, &frm)){
        printf("frame init error\n");
        return 0;
    }

    if(!read_nv12_frame(fp, &frm)){
        printf("Error during read\n");
        return 0;
    }

    fclose(fp);

    clock_t start = clock(), diff;
    int i,j,k;
    for(int k=0; k<FPS; k++){
        uint8_t* input = frm.lum;
        int16_t* output = out;

        for(j=0; j<HEIGHT/8; j++){
            for(i=0; i<WIDTH/8; i++){
                fwht(input, out);
                output += 8;
                input += 8;
             }
            output += WIDTH*7;
            input += WIDTH*7;
        }        
    } 
    diff = clock() - start;

    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("Time taken %d seconds %d milliseconds", msec/1000, msec%1000);
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
    return 0;   
}
