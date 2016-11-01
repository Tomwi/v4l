/*
 * Copyright 2016 Tom aan de Wiel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FRAME_H
#define FRAME_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct RAW_FRAME {
        unsigned int width, height;
        uint8_t *lum, *chrm;

} RAW_FRAME;

typedef struct FRAME_HEADER {
    unsigned int width, height;
    unsigned int lum_sz;
    unsigned int chroma_sz;
}FRAME_HEADER;

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
void writeRawFrame(FILE* fp, uint8_t* luminance, uint8_t* chroma, int width, int height);

int initCFrame(const unsigned int width, const unsigned int height, CFRAME* frm);
int readCFrame(FILE* fp, CFRAME* out);

#endif
