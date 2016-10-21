#include "decoder.h"


void addDeltas(int16_t* deltas, uint8_t *ref, int stride){
        for(int k=0; k<8; k++) {
                for(int l=0; l<8; l++) {
                        *deltas += *ref++;
                        if(*deltas < 0) {
                                *deltas = 0;
                        }
                        else if(*deltas > 255) {
                                *deltas = 255;
                        }
                        deltas++;
                }
                ref += stride-8;
                deltas += stride-8;
        }
}

void decodeFrame(CFRAME* frm, uint8_t* chref, uint8_t* lref, int16_t* chromaout, int16_t* lumaout){

        int i,j;

        int16_t* rlco = frm->rlc_data_chrm;
        int16_t* derlco = frm->chrm_coeff;
        int16_t* defwht = chromaout;

        for(j=0; j<frm->height/8; j++) {
                for(i=0; i<frm->width/2/8; i++) {
                        int stat = derlc(&rlco, derlco, frm->width/2);
                        // inverse transform them
                        //printf("pbit: %d\n", stat);
                        ifwht(derlco, defwht, WIDTH/2, WIDTH/2, (stat & PFRAME_BIT) ? 0 : 1);
                        // This block was P-coded
                        if(stat & PFRAME_BIT) {
                                // add reference to it.
                                uint8_t* refp = chref + j*8*frm->width/2+ i*8;
                                addDeltas(defwht, refp, frm->width/2);
                        }
                        // fwht outputs a block of 8 coefficients, so does derlc
                        derlco += 8;
                        // and for the decoder we need to shift our "
                        defwht += 8;
                }
                derlco += (frm->width/2)*7;
                defwht += (frm->width/2)*7;
        }

        rlco = frm->rlc_data_lum;
        derlco = frm->lum_coeff;
        defwht = lumaout;

        for(j=0; j<frm->height/8; j++) {
                for(i=0; i<frm->width/8; i++) {
                        int stat = derlc(&rlco, derlco, frm->width);
                        // inverse transform them
                        ifwht(derlco, defwht, WIDTH, WIDTH, (stat & PFRAME_BIT) ? 0 : 1);
                        // This block was P-coded
                        if(stat & PFRAME_BIT) {
                                // add reference to it.
                                uint8_t* refp = lref + j*8*frm->width+ i*8;
                                addDeltas(defwht, refp, frm->width);
                        }
                        // fwht outputs a block of 8 coefficients, so does derlc
                        derlco += 8;
                        // and for the decoder we need to shift our "
                        defwht += 8;
                }
                derlco += (frm->width)*7;
                defwht += (frm->width)*7;
        }
}
