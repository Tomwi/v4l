#include "encoder.h"

void fillBlock(uint8_t* input, int16_t* dst, int stride){
        int i,j;
        for(i=0; i<8; i++) {
                for(j=0; j<8; j++) {
                        *dst++  = *input++;
                }
                input += stride-8;
        }
}

/* Computes the sample variance */
int var(int16_t* input){

        int32_t mean=0;
        int32_t ssum=0;
        int i;
        for(i=0; i<8*8; i++, input++) {
                mean += *input;
                ssum += *input * *input;
        }
        return ssum - (mean*mean)/(64);
}

int decide_blocktype(uint8_t* current, uint8_t* reference, int16_t* deltablock, int stride){
        //return IBLOCK;
        int16_t tmp[64];
        fillBlock(current, tmp,stride);
        int varc = var(tmp);
        int k,l;
        int16_t* work = tmp;
        for(int k=0; k<8; k++) {
                for(int l=0; l<8; l++) {

                        *deltablock = *work - *reference;

                        deltablock++;
                        work++;
                        reference++;
                }
                reference += stride-8;
        }
        deltablock-=64;
        int vard = var(deltablock);
        return (varc <= vard ? IBLOCK : PBLOCK);
}

void encodeFrame(RAW_FRAME* frm, uint8_t* lref, uint8_t* cref, CFRAME* out){
        int i,j;

        /* TODO: make encoder state structure? */
        static int pchain_chrm = 0;
        static int pchain_lum = 0;

        int16_t deltablock[64];

        // encode chroma plane
        uint8_t* input = frm->chrm;

        int16_t* coeffs = out->chrm_coeff;
        int16_t* rlco = out->rlc_data_chrm;

        int waspcoded_chrm = 0;

        for(j=0; j<frm->height/8; j++) {
                for(i=0; i<frm->width/2/8; i++) {
                        int blocktype;
                        if(cref==NULL || pchain_chrm == MAX_PCHAIN ||
                           (blocktype=decide_blocktype(input, cref+(int)(input-frm->chrm), deltablock, frm->width/2))==IBLOCK) {
                                fwht(input, coeffs, frm->width/2, frm->width/2, 1);
                                blocktype = IBLOCK;

                        }
                        else{
                                //  blocktype = PBLOCK;
                                waspcoded_chrm = 1;
                                fwht16(deltablock, coeffs, 8, WIDTH/2, 0);

                        }

                        int ret = rlc(coeffs, rlco, frm->width/2, blocktype);
                        rlco += ret;
                        // advance to next block in current row
                        coeffs += 8;
                        input += 8;
                }
                // advance to next row, since chroma is subsampled, divide by 2
                coeffs += (frm->width/2)*7;
                input  += (frm->width/2)*7;
        }
        // size in bytes
        out->chroma_sz = (unsigned long)rlco - (unsigned long)out->rlc_data_chrm;

        if(pchain_chrm == MAX_PCHAIN)
                pchain_chrm = 0;
        /* Increase pchain count */
        if(waspcoded_chrm)
                pchain_chrm++;

        /* INTER FRAME */
        input = frm->lum;
        coeffs = out->lum_coeff;
        rlco = out->rlc_data_lum;

        int waspcoded = 0;
        for(j=0; j<frm->height/8; j++) {
                for(i=0; i<frm->width/8; i++) {
                        // intra code, first frame is always intra coded.
                        int blocktype;
                        if(lref==NULL || pchain_lum == MAX_PCHAIN ||
                           (blocktype=decide_blocktype(input, lref+(input-frm->lum), deltablock, frm->width))==IBLOCK) {
                                fwht(input, coeffs, frm->width, frm->width, 1);
                                blocktype = IBLOCK;
                        }
                        // inter code
                        else{
                                waspcoded = 1;
                                fwht16(deltablock, coeffs, 8, frm->width, 0);
                        }
                        int ret = rlc(coeffs, rlco, frm->width, blocktype);
                        /* Update rlc output block-pointers */
                        rlco += ret;
                        coeffs += 8;
                        input += 8;
                }
                coeffs += WIDTH*7;
                input += WIDTH*7;
        }
        out->lum_sz = (unsigned long)rlco - (unsigned long)out->rlc_data_lum;
        if(pchain_lum == MAX_PCHAIN)
                pchain_lum = 0;
        /* Increase pchain count */
        if(waspcoded)
                pchain_lum++;
}
