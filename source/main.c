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
#define FPS (900)

#define MAX_PCHAIN (10)

//#define WRITE

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

int16_t out_derlc[WIDTH*HEIGHT];
int16_t out_dec[WIDTH*HEIGHT];
uint8_t out_final[WIDTH*HEIGHT];

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

inline void addDeltas(int16_t* deltas, uint8_t *ref, uint8_t* input){
   for(int k=0; k<8; k++){
        for(int l=0; l<8; l++){

//            printf("(%d,%d, %d),", *deltas+*ref, *input, *deltas); 
            *deltas += *ref++;
            if(*deltas < 0){
                *deltas = 0;
            }
            else if(*deltas > 255){
                *deltas = 255;
            }
            deltas++;
            input++;
        }
 //       printf("\n");
        input += WIDTH-8;
        ref += WIDTH-8;
        deltas += WIDTH-8;
    }
}


int itra_dec(uint8_t* current, uint8_t* reference, int16_t* deltablock, int trigger){
    //return 1;
    int16_t tmp[64];
    fillBlock(current, tmp);
    int varc = var(tmp);
#if 0
    if(trigger==143)
        printf("DELTA CALC\n");
#endif
    int k,l;
    int16_t* work = tmp;
    for(int k=0; k<8; k++){
        for(int l=0; l<8; l++){

            *deltablock = *work - *reference;
#if 0
            if(trigger == 143){
                printf("(%d,%d,%d),", *work, *reference, *deltablock); 
            }
#endif
            deltablock++;
            work++;
            reference++;
        }
#if 0        
        if(trigger==143)
            printf("\n");
#endif
        reference += WIDTH-8;
    }
    deltablock-=64;
    int vard = var(deltablock);
    return (varc <= vard ? 1 : 0);
}



static _Bool init_app(const char * name, SDL_Surface * icon, uint32_t flags)
{
    atexit(SDL_Quit);
    if(SDL_Init(flags) < 0)
        return 0;

    SDL_WM_SetCaption(name, name);
    SDL_WM_SetIcon(icon, NULL);

    return 1;
}

static uint8_t * init_data(uint8_t * data)
{
    for(size_t i = WIDTH * HEIGHT * 3; i--; )
        data[i] = (i % 3 == 0) ? (i / 3) % WIDTH :
            (i % 3 == 1) ? (i / 3) / WIDTH : 0;

    return data;
}

static _Bool process(uint8_t * data)
{
    for(SDL_Event event; SDL_PollEvent(&event);)
        if(event.type == SDL_QUIT) return 0;

    for(size_t i = 0; i < WIDTH * HEIGHT * 3; i += 1 + rand() % 3)
        data[i] -= rand() % 8;

    return 1;
}

static void render(SDL_Surface * sf)
{
    SDL_Surface * screen = SDL_GetVideoSurface();
    if(SDL_BlitSurface(sf, NULL, screen, NULL) == 0)
        SDL_UpdateRect(screen, 0, 0, 0, 0);
}

static int filter(const SDL_Event * event)
{ return event->type == SDL_QUIT; }

#define mask32(BYTE) (*(uint32_t *)(uint8_t [4]){ [BYTE] = 0xff })

int main(int argc, char** argv){

	 (void)argc, (void)argv;
    static uint8_t buffer[WIDTH * HEIGHT * 3];

    _Bool ok =
        init_app("V4L WHT CODEC", NULL, SDL_INIT_VIDEO);

         SDL_Surface *screen = SDL_SetVideoMode(WIDTH, HEIGHT, 8, SDL_HWPALETTE);

    assert(ok);

	SDL_Color colors[256];
int d;

for(d = 0; d < 256; d++)
{
    colors[d].r = colors[d].g = colors[d].b = d;
}



    SDL_Surface * data_sf = SDL_CreateRGBSurfaceFrom(
        out_final, WIDTH, HEIGHT, 8, WIDTH,
        mask32(0), mask32(1), mask32(2), 0);

	SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);
    SDL_SetEventFilter(filter);

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
    int pcount = 0;
    fseek(fp, (WIDTH*HEIGHT + WIDTH*HEIGHT/2)*0, SEEK_SET);

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
        int16_t* derlco = out_derlc;
        int16_t* defwht = out_dec;

        int cnt = 0;
    
        int waspcoded = 0;
        for(j=0; j<HEIGHT/8; j++){
            for(i=0; i<WIDTH/8; i++){
                // intra code, first frame is always intra coded.
                int iframe;
                if((iframe=itra_dec(input, reference+(int)(input-frm.lum), deltablock, i)) || k==0 || pchain == MAX_PCHAIN){
                    iframe = 1;
                    fwht(input, output, WIDTH, WIDTH, 1);
                }
                // inter code
                else{
                    pcount--;
                    waspcoded = 1;
                    fwht16(deltablock, output, 0, WIDTH, 0);
                }
              
                int ret = rlc(output, rlco, WIDTH, iframe ? 0 : 1);               
                // decompress freshly coded coefficients
                int stat = derlc(rlco, derlco, WIDTH);
                // inverse transform them
                ifwht(derlco, defwht, WIDTH, WIDTH, iframe);
               
      
                // This block was P-coded
#if 1 
                if(iframe==0){
                    // add refernce to it.                    
                    uint8_t* refp = reference + j*8*WIDTH+ i*8;
                    addDeltas(defwht, refp, input);
                    pcount++;
                }
#endif
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
        printf("Compression ratio %lf \%\n",
               ((float)(WIDTH*HEIGHT))/(rlco - out_rlc)*100); 
    	int a;
        for(a=0; a<WIDTH*HEIGHT; a++)
            out_final[a] = (uint8_t)out_dec[a];
SDL_Delay(40);
 	render(data_sf);
#ifdef WRITE
    

            fwrite(out_final, 1, sizeof(uint8_t)*WIDTH*HEIGHT, fp2);
#endif 
            uint16_t* swp = reference;
            reference = frm.lum;
           frm.lum = swp;
    } 
    diff = clock() - start;

    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
    printf("pframes balanced ? %d\n", pcount);
#if 1
    int16_t*    print = out_derlc;
    int8_t* print2 = frm.lum;
    int16_t* print3 = out_dec;

    for(j=0; j<8; j++){
        for(i=0; i<8; i++){
            printf("(%d vs %d, %d),", *print++, *print3++, *print2++);
        }
        print+= WIDTH-8;
        print2 += WIDTH-8;
        print3 += WIDTH-8;
        printf("\n");
    }
#endif
    fclose(fp);

#ifdef WRITE
    fclose(fp2);
#endif

   return 0;   
}
