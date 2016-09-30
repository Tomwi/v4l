#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fwht.h"

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

    if(argc <= 0){
        printf("please provide input and output\n");
    }

    FILE* fp = fopen(argv[1], "wb");
    if(fp == NULL){
        printf("Failed to open %s\n", argv[1]);
        return 0;
    }
    // 
//    fseek(frame
    fread(frame, 1, sizeof(uint8_t)*WIDTH*HEIGHT, fp);
    fclose(fp);

    clock_t start = clock(), diff;
    int i,j,k;
    for(int k=0; k<FPS; k++){
        uint8_t* input = frame;
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

    return 0;   
}
