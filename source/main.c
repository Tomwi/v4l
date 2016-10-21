#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>
#include <assert.h>

#include "fwht.h"
#include "frame.h"
#include "rlc.h"
#include "encoder.h"
#include "decoder.h"

#include "params.h"

#define FPS (100)

int16_t chroma[WIDTH*HEIGHT/2], luminance[WIDTH*HEIGHT];
int8_t chroma_8bit[WIDTH*HEIGHT/2], luminance_8bit[WIDTH*HEIGHT];

int main(int argc, char** argv){
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

        RAW_FRAME raw_frm;
        CFRAME cfrm_current, cfrm_reference;

        if(argc <= 1) {
                printf("please provide input and output\n");
                return 0;
        }

        FILE* fp = fopen(argv[1], "rb");

#ifdef WRITE
        FILE* fp2 = fopen("test output", "wb");
#endif
        if(fp == NULL) {
                printf("Failed to open %s\n", argv[1]);
                return 0;
        }
        if(!initRawFrame(WIDTH, HEIGHT, &raw_frm) || !initCFrame(WIDTH, HEIGHT, &cfrm_current) || !initCFrame(WIDTH, HEIGHT, &cfrm_current)) {
                printf("frame init error\n");
                return 0;
        }

        clock_t start = clock(), diff;
        for(int k=0; k<FPS; k++) {
                if(!readRawFrame(fp, &raw_frm)) {
                        printf("Error during read\n");
                        return 0;
                }
                if(k==0)
                        encodeFrame(&raw_frm, NULL, NULL, &cfrm_current);
                else
                        encodeFrame(&raw_frm, luminance_8bit, chroma_8bit, &cfrm_current);

                decodeFrame(&cfrm_current, chroma_8bit, luminance_8bit, chroma, luminance);

                printf("%lf and %lf\n", (float)WIDTH*HEIGHT/cfrm_current.lum_sz * 100, (float)WIDTH*HEIGHT/2/cfrm_current.chroma_sz * 100);
                int a;
                for(a=0; a<WIDTH*HEIGHT; a++)
                        luminance_8bit[a] = (uint8_t)luminance[a];
                //  int a;
                for(a=0; a<(WIDTH*HEIGHT/2); a++)
                        chroma_8bit[a] = (uint8_t)chroma[a];
                SDL_Rect rect;

                SDL_LockYUVOverlay(bmp);
                memcpy(bmp->pixels[0], luminance_8bit, WIDTH*HEIGHT);
                memcpy(bmp->pixels[2],chroma_8bit, WIDTH*HEIGHT/4);
                memcpy(bmp->pixels[1], chroma_8bit +WIDTH*HEIGHT/4, WIDTH*HEIGHT/4);

                SDL_UnlockYUVOverlay(bmp);

                rect.x = 0;
                rect.y = 0;
                rect.w = WIDTH;
                rect.h = HEIGHT;
                SDL_DisplayYUVOverlay(bmp, &rect);        

        }
        diff = clock() - start;

        int msec = diff * 1000 / CLOCKS_PER_SEC;
        printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
        fclose(fp);

#ifdef WRITE
        fclose(fp2);
#endif

        return 0;
}
