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
        if(fp == NULL) {
                return 0;
        }
        /* TODO: implement frame header */
        return 1;
}
