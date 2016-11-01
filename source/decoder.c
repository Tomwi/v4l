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

#include "decoder.h"


void addDeltas(int16_t* deltas, uint8_t *ref, int stride){
        for(int k=0; k<8; k++) {
                for(int l=0; l<8; l++) {
                        *deltas += *ref++;
                        /* Due to quantizing, it might possible that the
                        * decoded coefficients are slightly out of range
                        */
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
                        ifwht(derlco, defwht, WIDTH/2, WIDTH/2, (stat & PFRAME_BIT) ? 0 : 1);
                        if(stat & PFRAME_BIT) {
                                // add deltas
                                uint8_t* refp = chref + j*8*frm->width/2+ i*8;
                                addDeltas(defwht, refp, frm->width/2);
                        }
                        // advance to next column
                        derlco += 8;
                        defwht += 8;
                }
                // advance to next row
                derlco += (frm->width/2)*7;
                defwht += (frm->width/2)*7;
        }
        rlco = frm->rlc_data_lum;
        derlco = frm->lum_coeff;
        defwht = lumaout;

        for(j=0; j<frm->height/8; j++) {
                for(i=0; i<frm->width/8; i++) {
                        int stat = derlc(&rlco, derlco, frm->width);
                        ifwht(derlco, defwht, WIDTH, WIDTH, (stat & PFRAME_BIT) ? 0 : 1);
                        if(stat & PFRAME_BIT) {
                                // add deltas
                                uint8_t* refp = lref + j*8*frm->width+ i*8;
                                addDeltas(defwht, refp, frm->width);
                        }
                        derlco += 8;
                        defwht += 8;
                }
                derlco += (frm->width)*7;
                defwht += (frm->width)*7;
        }
}
