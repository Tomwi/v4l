#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>
#include <assert.h>

#include "fwht.h"
#include "nv12.h"
#include "rlc.h"
#include "encoder.h"
#include "decoder.h"

#include "params.h"

#define FPS (940)

#define MAX_PCHAIN (10)
//#define DEBUG
//#define DEBUG_DELTA
//#define WRITE

// reference frame for the encoder
uint8_t ref_frame[WIDTH*HEIGHT];
uint8_t ref_frame_chrm[WIDTH*HEIGHT];

// reference frame for the decoder
uint8_t ref_frame_dec[WIDTH*HEIGHT];
uint8_t ref_frame_chrm_dec[WIDTH*HEIGHT/2];

int16_t out[WIDTH*HEIGHT];
int16_t out_chrm[WIDTH*HEIGHT];

int16_t out_rlc[WIDTH*HEIGHT+WIDTH*HEIGHT/64]; // + headers for each block.
int16_t out_rlc_chrm[WIDTH*HEIGHT/2+WIDTH*HEIGHT/64/2];

int16_t out_derlc[WIDTH*HEIGHT];
int16_t out_derlc_chrm[WIDTH*HEIGHT/2];

int16_t out_dec[WIDTH*HEIGHT];
int16_t out_dec_chrm[WIDTH*HEIGHT/2];

uint8_t out_final[WIDTH*HEIGHT];
uint8_t out_final_chrm[WIDTH*HEIGHT/2];

int main(int argc, char** argv){

        (void)argc, (void)argv;
        static uint8_t buffer[WIDTH * HEIGHT * 3];


        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
                fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
                exit(1);
        }

        SDL_Surface *screen;

        screen = SDL_SetVideoMode(WIDTH, HEIGHT, 0, 0);
        if(!screen) {
                fprintf(stderr, "SDL: could not set video mode - exiting\n");
                exit(1);
        }
        SDL_Overlay     *bmp = NULL;

        bmp = SDL_CreateYUVOverlay(WIDTH, HEIGHT,
                                   SDL_YV12_OVERLAY, screen);

        FRAME frm;
        // if(argc <= 1) {
        //         printf("please provide input and output\n");
        //         return 0;
        // }
        //int a = getchar();
        FILE* fp = fopen("../../../../tomwi/Videos/yuv420.yuv", "rb"); //fopen(argv[1], "rb");
#ifdef WRITE
        FILE* fp2 = fopen("test output", "wb");
#endif
        if(fp == NULL) {
                printf("Failed to open %s\n", argv[1]);
                return 0;
        }
        if(!init_nv12_frame(WIDTH, HEIGHT, &frm)) {
                printf("frame init error\n");
                return 0;
        }

        clock_t start = clock(), diff;
        int i,j,k;
        int pcount = 0;
        fseek(fp, (WIDTH*HEIGHT + WIDTH*HEIGHT/2)*0, SEEK_SET);

        uint8_t* reference_chrm = out_final_chrm;

        int16_t deltablock[64];
        int pchain=0;
        int pchain_chrm = 0;

        for(int k=0; k<FPS; k++) {
                if(!read_nv12_frame(fp, &frm)) {
                        printf("Error during read\n");
                        return 0;
                }

                uint8_t* input = frm.chrm;
                int16_t* output = out_chrm;
                int16_t* rlco = out_rlc_chrm;
                int16_t* derlco = out_derlc_chrm;
                int16_t* defwht = out_dec_chrm;

                // encode Chroma planes, those are are subsampled (4:2:2)
                // We encode the two consecutive chroma planes as if they were
                // one plane. This assumes width/2 and height/2 to be divisible by 8
                 int waspcoded_chrm = 0;
#if 1
                 for(j=0; j<HEIGHT/8; j++) {
                         for(i=0; i<WIDTH/2/8; i++) {
                                int iframe;
                                if((iframe=itra_dec(input, out_final_chrm+(int)(input-frm.chrm), deltablock, WIDTH/2)) || k==0 || pchain_chrm == MAX_PCHAIN) {
                                        iframe = 1;
                                        fwht(input, output, WIDTH/2, WIDTH/2, 1);
                                }
                                //inter code
                                else{
                                      //printf("p coded frame @ %d %d\n", i, j);
                                        waspcoded_chrm = 1;
                                        fwht16(deltablock, output, 8, WIDTH/2, 0);

                                }
                                //printf("%d %d\n", i,j);
                                int ret = rlc(output, rlco, WIDTH/2, iframe ? 0 : 1);
                                // decompress freshly coded coefficients
                                int stat = derlc(rlco, derlco, WIDTH/2);
                                // inverse transform them
                                ifwht(derlco, defwht, WIDTH/2, WIDTH/2, iframe);


                                // This block was P-coded
                                if(iframe==0) {
                                        // add refernce to it.
                                        uint8_t* refp = out_final_chrm + j*8*WIDTH/2+ i*8;
                                        addDeltas(defwht, refp, input, WIDTH/2);
                                }
                                /* Update rlc output block-pointers */
                                rlco += ret;
                                // fwht outputs a block of 8 coefficients, so does derlc
                                output += 8;
                                derlco += 8;
                                // so we shift our input by the same amount
                                input += 8;
                                // and for the decoder we need to shift our "
                                defwht += 8;
                        }
                        derlco += (WIDTH/2)*7;
                        output += (WIDTH/2)*7;
                        input += (WIDTH/2)*7;
                        defwht += (WIDTH/2)*7;
                }
#endif
                if(pchain_chrm == MAX_PCHAIN)
                        pchain_chrm = 0;
                /* Increase pchain count */
                if(waspcoded_chrm)
                        pchain_chrm++;
                        printf("Compression ratio chroma %lf %p %p %d %d \n",
                               ((float)(WIDTH*HEIGHT/2))/((unsigned long)rlco - (unsigned long)out_rlc_chrm)*100, rlco, out_rlc_chrm, 2*(rlco-out_rlc_chrm), WIDTH*HEIGHT/2);
#if 1

#ifdef WRITE
                fwrite(out_rlc_chrm, 1, sizeof(uint16_t)*(rlco - out_rlc_chrm), fp2);
#endif
                /*
                 *  ENCODE INTER frame
                 */
                input = frm.lum;
                output = out;
                rlco = out_rlc;
                derlco = out_derlc;
                defwht = out_dec;

                int cnt = 0;

                int waspcoded = 0;
                for(j=0; j<HEIGHT/8; j++) {
                        for(i=0; i<WIDTH/8; i++) {
                                // intra code, first frame is always intra coded.
                                int iframe;
                                if((iframe=itra_dec(input, out_final+(int)(input-frm.lum), deltablock, WIDTH)) || k==0 || pchain == MAX_PCHAIN) {
                                        iframe = 1;
                                        fwht(input, output, WIDTH, WIDTH, 1);
                                }
                                // inter code
                                else{
                                        waspcoded = 1;
                                        fwht16(deltablock, output, 8, WIDTH, 0);

                                }
                                int ret = rlc(output, rlco, WIDTH, iframe ? 0 : 1);
                                // decompress freshly coded coefficients
                                int stat = derlc(rlco, derlco, WIDTH);
                                // inverse transform them
                                ifwht(derlco, defwht, WIDTH, WIDTH, iframe);


                                // This block was P-coded
                                if(iframe==0) {
                                        // add refernce to it.
                                        uint8_t* refp = out_final + j*8*WIDTH+ i*8;
                                        addDeltas(defwht, refp, input, WIDTH);
                                }
                                /* Update rlc output block-pointers */
                                rlco += ret;
                                // fwht outputs a block of 8 coefficients, so does derlc
                                output += 8;
                                derlco += 8;
                                // so we shift our input by the same amount
                                input += 8;
                                // and for the decoder we need to shift our "
                                defwht += 8;
                        }
                        derlco += WIDTH*7;
                        output += WIDTH*7;
                        input += WIDTH*7;
                        defwht += WIDTH*7;
                }
                if(pchain == MAX_PCHAIN)
                        pchain = 0;
                /* Increase pchain count */
                if(waspcoded)
                        pchain++;
                printf("Compression ratio luma %lf \% \n",
                       ((float)(WIDTH*HEIGHT))/((unsigned long)rlco - (unsigned long)out_rlc)*100);

                        #ifdef WRITE
                                        fwrite(out_rlc, 1, ((unsigned long)rlco - (unsigned long)out_rlc), fp2);
                        #endif
                int a;
                for(a=0; a<WIDTH*HEIGHT; a++)
                        out_final[a] = (uint8_t)out_dec[a];
                        #endif
                      //  int a;
                for(a=0; a<(WIDTH*HEIGHT/2); a++)
                        out_final_chrm[a] = (uint8_t)out_dec_chrm[a];
                SDL_Rect rect;

                SDL_LockYUVOverlay(bmp);
                memcpy(bmp->pixels[0], out_final, WIDTH*HEIGHT);
                memcpy(bmp->pixels[2],out_final_chrm, WIDTH*HEIGHT/4);
                memcpy(bmp->pixels[1], out_final_chrm+WIDTH*HEIGHT/4, WIDTH*HEIGHT/4);

                SDL_UnlockYUVOverlay(bmp);

                rect.x = 0;
                rect.y = 0;
                rect.w = WIDTH;
                rect.h = HEIGHT;
                SDL_DisplayYUVOverlay(bmp, &rect);
                //getchar();
        }
        diff = clock() - start;

        int msec = diff * 1000 / CLOCKS_PER_SEC;
        printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
        printf("pframes balanced ? %d\n", pcount);
        fclose(fp);

#ifdef WRITE
        fclose(fp2);
#endif

        return 0;
}
