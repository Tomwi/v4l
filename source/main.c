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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>
#include <assert.h>

#include "fwht.h"
#include "frame.h"
#include "rlc.h"
#include "encoder.h"
#include "decoder.h"

#include "params.h"
//#define WRITE
//#define READ
//#define RES_CHANGE
//#define RES_CHANGE_FRAME (50)
//#define RES_CHANGE_SCALE_DIV (2)
#define STATELESS
#define FPS (500)
// #define WRITE_RAW
// #define WRITE_CRATIO
// #define WRITE_TIMES
// #define TIME_ENCODER
// #define WRITE_PCOUNT
int16_t chroma[WIDTH*HEIGHT/2], luminance[WIDTH*HEIGHT];
int8_t chroma_8bit[WIDTH*HEIGHT/2], luminance_8bit[WIDTH*HEIGHT];
int pcount[2];

int main(int argc, char **argv)
{
	ENCODER enc;
	#ifndef RES_CHANGE
	if (argc <= 3) {
		printf("please provide input file, width and height\n");
		return 0;
	}
	#else
	if (argc <= 6) {
		printf("please provide input file, width and height, second input file, width and height\n");
		return 0;
	}
	#endif

	int width, height;
	sscanf(argv[2], "%d",  &width);
	sscanf(argv[3], "%d",  &height);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}
	SDL_Surface *screen;

	screen = SDL_SetVideoMode(width, height, 0, 0);
	if (!screen) {
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}
	SDL_Overlay *bmp = SDL_CreateYUVOverlay(width, height,
				   SDL_YV12_OVERLAY, screen);

	RAW_FRAME raw_frm;
	ENCODER_META enc_meta;
	DECODER_META dec_meta;

	CFRAME cfrm_current, cfrm_reference;

	FILE *fp = fopen(argv[1], "rb");

#ifdef WRITE
	FILE *fp2 = fopen("/run/media/tomwi/data/compressedoutput", "wb");
#endif

#ifdef WRITE_RAW
	FILE *fp3 = fopen("/run/media/tomwi/data/rawoutput", "wb");
#endif

#ifdef WRITE_CRATIO
	FILE *fp4 = fopen("/run/media/tomwi/data/cratio.txt", "wb");
#endif

#ifdef WRITE_TIMES
	FILE *fp5 = fopen("/run/media/tomwi/data/times.txt", "wb");
#endif
#ifdef WRITE_PCOUNT
	FILE* fp6 = fopen("/run/media/tomwi/data/pcount.txt", "wb");
#endif


	if (fp == NULL) {
		printf("Failed to open %s\n", argv[1]);
		return 0;
	}
	if (!initRawFrame(width, height, &raw_frm) || !initCFrame(width, height, &cfrm_current)) {
		printf("frame init error\n");
		return 0;
	}



	for (int k = 0; k < FPS; k++) {

		#ifdef RES_CHANGE
		if(k==RES_CHANGE_FRAME){
			fclose(fp);
			destroyRawFrame(&raw_frm);
			destroyCFrame(&cfrm_current);

			fp = fopen(argv[4], "rb");

			sscanf(argv[5], "%d",  &width);
			sscanf(argv[6], "%d",  &height);
			/* Reinitialize structures and SDL */
			if (!initRawFrame(width, height, &raw_frm) || !initCFrame(width, height, &cfrm_current)) {
				printf("frame init error\n");
				return 0;
			}
			screen = SDL_SetVideoMode(width, height, 0, 0);
			if (!screen) {
				fprintf(stderr, "SDL: could not set video mode - exiting\n");
				exit(1);
			}

			bmp = SDL_CreateYUVOverlay(width, height,
						   SDL_YV12_OVERLAY, screen);
		}

		#endif
		#ifndef READ
		if (!readRawFrame(fp, &raw_frm)) {
			printf("Error during read\n");
			return 0;
		}
#if 0
		if(k==101){
				FILE* fps = fopen("original.yuv", "wb");
				writeRawFrame(fps, raw_frm.lum, raw_frm.chrm, width, height);
				fclose(fps);
		}
#endif
			clock_t start = clock(), diff;
		pcount[0] = 0;
		pcount[1] =0;

		#ifdef STATELESS
		enc.prev_resolution[0] = enc.cur_resolution[0];
		enc.prev_resolution[1] = enc.cur_resolution[1];
		enc.cur_resolution[0] = raw_frm.width;
		enc.cur_resolution[1] = raw_frm.height;
		enc.max_pchain = MAX_PCHAIN;
		// enc.quant_intra = QUANT_TABLE;
		// enc.quantizeInter = QUANT_TABLE_P;
		enc.chrm = raw_frm.chrm;
		enc.luma = raw_frm.lum;

		enc_meta.rlc_data_chrm = cfrm_current.rlc_data_chrm;
		enc_meta.rlc_data_lum = cfrm_current.rlc_data_lum;
		enc_meta.chrm_coeff = cfrm_current.chrm_coeff;
		enc_meta.lum_coeff = cfrm_current.lum_coeff;

		if (k == 0)
			encodeFrameStateless(&enc, NULL, NULL, &enc_meta);
		else
			encodeFrameStateless(&enc, luminance_8bit, chroma_8bit, &enc_meta);

		// construct frame header
		cfrm_current.chroma_sz = enc_meta.chroma_sz;
		cfrm_current.lum_sz = enc_meta.lum_sz;

		#else
		if (k == 0)
			encodeFrame(&raw_frm, NULL, NULL, &cfrm_current, pcount);
		else
			encodeFrame(&raw_frm, luminance_8bit, chroma_8bit, &cfrm_current, pcount);
		#endif

			#ifdef TIME_ENCODER
			int i,j;
			int16_t *rlco = cfrm_current.rlc_data_chrm;
			int stat = *rlco;

			int16_t *input = cfrm_current.chrm_coeff;
			int16_t *coeffs = chroma;
			uint8_t *chref = chroma_8bit;

			/* Only for timing purposes!!! */
			for (j = 0; j < height/8; j++) {
				for (i = 0; i < width/2/8; i++) {
					ifwht(input, coeffs, width/2, width/2, (stat & PFRAME_BIT) ? 0 : 1);
					if (stat & PFRAME_BIT) {
						// add deltas
						uint8_t *refp = chref + j*8*width/2 + i*8;
						addDeltas(coeffs, refp, width/2);
					}
					coeffs += 8;
					input += 8;
				}
				coeffs += (width/2)*7;
				input  += (width/2)*7;
			}

			rlco = cfrm_current.rlc_data_lum;
			stat = *rlco;

			input = cfrm_current.lum_coeff;
			coeffs = chroma;
			uint8_t* lref = luminance_8bit;

			/* Only for timing purposes!!! */
			for (j = 0; j < height/8; j++) {
				for (i = 0; i < width/8; i++) {
					ifwht(input, coeffs, width, width, (stat & PFRAME_BIT) ? 0 : 1);
					if (stat & PFRAME_BIT) {
						// add deltas
						uint8_t *refp = lref + j*8*width + i*8;
						addDeltas(coeffs, refp, width);
					}
					coeffs += 8;
					input += 8;
				}
				coeffs += (width)*7;
				input  += (width)*7;
			}
			#endif
			diff = clock() - start;
		int msec = diff; // * 1000 / CLOCKS_PER_SEC;

		#ifdef WRITE
			writeCFrame(&cfrm_current, fp2);
		#endif
		#else
			readCFrame(fp, &cfrm_current);
		#endif

		clock_t start2 = clock(), diff2;
		#ifdef STATELESS
		dec_meta.rlc_data_chrm = cfrm_current.rlc_data_chrm;
		dec_meta.rlc_data_lum = cfrm_current.rlc_data_lum;
		dec_meta.chrm_coeff = cfrm_current.chrm_coeff;
		dec_meta.lum_coeff = cfrm_current.lum_coeff;
		dec_meta.width = cfrm_current.width;
		dec_meta.height = cfrm_current.height;
			decodeFrame(&dec_meta, chroma_8bit, luminance_8bit, chroma, luminance);
		#else
			decodeFrame(&cfrm_current, chroma_8bit, luminance_8bit, chroma, luminance);
		#endif
		diff2 = clock() - start2;

		int msec2 = diff2; // * 1000 / CLOCKS_PER_SEC;
	//	printf("%d %d\n", enc.cur_resolution[0], enc.cur_resolution[1]);
	//	printf("read %d %d\n", cfrm_current.lum_sz, cfrm_current.chroma_sz);
	//	printf("%lf and %lf\n", (float)width*height/cfrm_current.lum_sz * 100, (float)width*height/2/cfrm_current.chroma_sz * 100);
		#ifdef WRITE_CRATIO
		fprintf(fp4, "%f, %f\n", (float)width*height/cfrm_current.lum_sz * 100, (float)width*height/2/cfrm_current.chroma_sz * 100);
		#endif

		#ifdef WRITE_TIMES
		fprintf(fp5, "%d, %d\n", msec, msec2);
		#endif
		#ifdef WRITE_PCOUNT
		fprintf(fp6,"%d,%d\n", pcount[0], pcount[1]);
		#endif

		int a;

		for (a = 0; a < width*height; a++)
			luminance_8bit[a] = (uint8_t)luminance[a];
		//  int a;
		for (a = 0; a < (width*height/2); a++)
			chroma_8bit[a] = (uint8_t)chroma[a];

			#ifdef WRITE_RAW
				writeRawFrame(fp3, luminance_8bit, chroma_8bit, width, height);
			#endif

		SDL_Rect rect;

		SDL_LockYUVOverlay(bmp);
		memcpy(bmp->pixels[0], luminance_8bit, width*height);
		memcpy(bmp->pixels[2], chroma_8bit, width*height/4);
		memcpy(bmp->pixels[1], chroma_8bit + width*height/4, width*height/4);

		SDL_UnlockYUVOverlay(bmp);

		rect.x = 0;
		rect.y = 0;
		rect.w = width;
		rect.h = height;
		SDL_DisplayYUVOverlay(bmp, &rect);
		// if(k==101){
		// 	FILE* fps = fopen("sample_frame.yuv", "wb");
		// 	writeRawFrame(fps, luminance_8bit, chroma_8bit, width, height);
		// 	fclose(fps);
		// 	getchar();
		// 	exit(EXIT_SUCCESS);
		// }

	}

	fclose(fp);

#ifdef WRITE
	fclose(fp2);
#endif
#ifdef WRITE_PCOUNT
	fclose(fp6);
#endif
	return 0;
}
