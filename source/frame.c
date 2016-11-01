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

#include "frame.h"

int initRawFrame(const unsigned int width, const unsigned int height, RAW_FRAME* frm){
        frm->width = width;
        frm->height = height;
        frm->lum = malloc(sizeof(uint8_t)*width*height);
        frm->chrm = malloc(sizeof(uint8_t)*width*height/2);
        if(frm->lum == NULL) {
                if(frm->chrm != NULL)
                        free(frm->chrm);
                return 0;
        }

        if(frm->chrm == NULL) {
                if(frm->lum !=NULL) {
                        free(frm->lum);
                }
                return 0;
        }
        return 1;
}

int readRawFrame(FILE* fp, RAW_FRAME* out){
        if(fp == NULL) {
                return 0;
        }
        int sz = out->width * out->height;
        int ret = fread(out->lum, 1, sz, fp);
        if(ret != sizeof(uint8_t)*sz) {
                return 0;
        }
        ret = fread(out->chrm, 1, sizeof(uint8_t)*sz/2, fp);
        if(ret != sizeof(uint8_t)*sz/2) {
                return 0;
        }
        return 1;
}

void writeRawFrame(FILE* fp, uint8_t* luminance, uint8_t* chroma, int width, int height){
        if(fp == NULL) {
                return;
        }
        int sz = width*height;
        int ret = fwrite(luminance, 1, sz, fp);
        if(ret != sizeof(uint8_t)*sz) {
                return;
        }
        ret = fwrite(chroma, 1, sizeof(uint8_t)*sz/2, fp);
        if(ret != sizeof(uint8_t)*sz/2) {
                return;
        }
}


int initCFrame(const unsigned int width, const unsigned int height, CFRAME* frm){

        frm->width = width;
        frm->height = height;
        // each RLC coefficient is 16 bits. Worst case every coefficient is non-zero
        // since each block has a 16 bit header, we allocate width*height/64 extra
        // to include space for all headers in the whole frame
        frm->rlc_data_lum = malloc(sizeof(int16_t)*(width*height + width*height/64));
        frm->rlc_data_chrm = malloc(sizeof(int16_t)*(width*height/2 + width*height/64/2));
        // FWHT COEFFICENTS, 16 bits accuracy
        frm->lum_coeff = malloc(sizeof(int16_t)*width*height);
        frm->chrm_coeff = malloc(sizeof(int16_t)*width*height/2);

        if(frm->rlc_data_lum == NULL || frm->rlc_data_chrm == NULL ||
           frm->lum_coeff == NULL || frm->chrm_coeff == NULL) {
                goto error_init_cframe;
        }
        return 1;

error_init_cframe:
        if(frm->rlc_data_lum)
                free(frm->rlc_data_lum);
        if(frm->rlc_data_chrm)
                free(frm->rlc_data_chrm);
        if(frm->chrm_coeff)
                free(frm->chrm_coeff);
        if(frm->lum_coeff)
                free(frm->lum_coeff);
        return 0;
}

int readCFrame(FILE* fp, CFRAME* out){
        // Assumes an already initialized CFRAME structure.
        if(fp == NULL || out->chrm_coeff == NULL || out->lum_coeff == NULL) {
                return 0;
        }
        FRAME_HEADER hdr;
        fread(&hdr, 1, sizeof(FRAME_HEADER), fp);
        out->lum_sz = hdr.lum_sz;
        out->chroma_sz = hdr.chroma_sz;
        fread(out->lum_coeff, 1, hdr.lum_sz, fp);
        fread(out->chrm_coeff, 1, hdr.chroma_sz, fp);
        return 1;
}

void writeCFrame(CFRAME* frm, FILE* fp){
  if(fp==NULL || frm==NULL)
    return;
  FRAME_HEADER hdr;
  hdr.width = frm->width;
  hdr.height = frm->height;
  hdr.lum_sz = frm->lum_sz;
  hdr.chroma_sz = frm->chroma_sz;
  fwrite(&hdr, 1, sizeof(FRAME_HEADER), fp);
  fwrite(frm->lum_coeff, 1, hdr.lum_sz,fp);
  fwrite(frm->chrm_coeff, 1, hdr.chroma_sz,fp);
}
