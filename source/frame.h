#ifndef NV12_H
#define NV12_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct RAW_FRAME {
        unsigned int width, height;
        uint8_t *lum, *chrm;

} RAW_FRAME;

typedef struct CFRAME {
        unsigned int width, height;
        int16_t* rlc_data_chrm;
        int16_t* rlc_data_lum;
        int16_t* chrm_coeff;
        int16_t* lum_coeff;
        // sizes of luma and chroma planes (compressed)
        unsigned int chroma_sz, lum_sz;
} CFRAME;

#define PBLOCK (0)
#define IBLOCK (1)

int initRawFrame(const unsigned int width, const unsigned int height, RAW_FRAME* frm);
int readRawFrame(FILE* fp, RAW_FRAME* out);

int initCFrame(const unsigned int width, const unsigned int height, CFRAME* frm);
int readCFrame(FILE* fp, CFRAME* out);

#endif
