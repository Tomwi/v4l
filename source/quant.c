#include "quant.h"


const int QUANT_TABLE[] = {
	2, 2, 2, 2, 2, 2,  2,  2,
	2, 2, 2, 2, 2, 2,  2,  2,
	2, 2, 2, 2, 2, 2,  2,  6,
	2, 2, 2, 2, 2, 2,  6,  6,
	2, 2, 2, 2, 2, 6,  6,  6,
	2, 2, 2, 2, 6, 6,  6,  6,
	2, 2, 2, 6, 6, 6,  6,  6,
	2, 2, 6, 6, 6, 6,  6,  6,

};

void quantize(int16_t* coeff, int stride){
	int* quant = QUANT_TABLE;
	int i;
	int16_t* tmp = coeff;
	for(i=0; i<8; i++){
		*tmp >>= *quant++;
		if(*tmp >= -10 && *tmp <= 10)
			*tmp = 0;

		tmp += stride;
	}
}

void deadzone_quant(int16_t* coeff, int stride){
	int i;
	int16_t* tmp = coeff;
	for(i=0; i<8; i++){
		*tmp >>= 3;
		if(*tmp >= -10 && *tmp <= 10)
			*tmp = 0;
		tmp += stride;
	}
}

void dequantize(int16_t* coeff, int stride){
	int* quant = QUANT_TABLE;
	int i;
	int16_t* tmp = coeff;
	for(i=0; i<8; i++){
		*tmp <<= *quant++;
		tmp += stride;
	}

}
