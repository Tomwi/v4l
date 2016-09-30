#ifndef NV12_H
#define NV12_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct FRAME {
    unsigned int width, height;
    uint8_t *lum, *chrm;
} FRAME;

int init_nv12_frame(const unsigned int width, const unsigned int height, FRAME* frm);
int read_nv12_frame(FILE* fp, const unsigned int width, const unsigned int height, FRAME* out);

#endif
