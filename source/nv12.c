#include "nv12.h"

int init_nv12_frame(const unsigned int width, const unsigned int height, FRAME* frm){
        frm->width = width;
        frm->height = height;
        frm->lum = malloc(sizeof(uint8_t)*width*height);
        frm->chrm = malloc(sizeof(uint8_t)*width*height);
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

int read_nv12_frame(FILE* fp, FRAME* out){
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
