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
#define FPS (500)
#define WRITE_RAW
#define WRITE_CRATIO
#define WRITE_TIMES
#define TIME_ENCODER
#define WRITE_PCOUNT
int16_t chroma[WIDTH*HEIGHT/2], luminance[WIDTH*HEIGHT];
int8_t chroma_8bit[WIDTH*HEIGHT/2], luminance_8bit[WIDTH*HEIGHT];
int pcount[2];

int main(int argc, char **argv)
{
	printf("clocks per sec %d\n", CLOCKS_PER_SEC);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}
	SDL_Surface *screen;

	screen = SDL_SetVideoMode(WIDTH, HEIGHT, 0, 0);
	if (!screen) {
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}
	SDL_Overlay     *bmp = NULL;

	bmp = SDL_CreateYUVOverlay(WIDTH, HEIGHT,
				   SDL_YV12_OVERLAY, screen);

	RAW_FRAME raw_frm;
	CFRAME cfrm_current, cfrm_reference;

	if (argc <= 1) {
		printf("please provide input and output\n");
		return 0;
	}

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
	if (!initRawFrame(WIDTH, HEIGHT, &raw_frm) || !initCFrame(WIDTH, HEIGHT, &cfrm_current)) {
		printf("frame init error\n");
		return 0;
	}



	for (int k = 0; k < FPS; k++) {
		#ifndef READ
		if (!readRawFrame(fp, &raw_frm)) {
			printf("Error during read\n");
			return 0;
		}
			clock_t start = clock(), diff;
		pcount[0] = 0;
		pcount[1] =0;
		if (k == 0)
			encodeFrame(&raw_frm, NULL, NULL, &cfrm_current, pcount);
		else
			encodeFrame(&raw_frm, luminance_8bit, chroma_8bit, &cfrm_current, pcount);

			#ifdef TIME_ENCODER
			int i,j;
			int16_t *rlco = cfrm_current.rlc_data_chrm;
			int stat = *rlco;

			int16_t *input = cfrm_current.chrm_coeff;
			int16_t *coeffs = chroma;
			uint8_t *chref = chroma_8bit;

			/* Only for timing purposes!!! */
			for (j = 0; j < HEIGHT/8; j++) {
				for (i = 0; i < WIDTH/2/8; i++) {
					ifwht(input, coeffs, WIDTH/2, WIDTH/2, (stat & PFRAME_BIT) ? 0 : 1);
					if (stat & PFRAME_BIT) {
						// add deltas
						uint8_t *refp = chref + j*8*WIDTH/2 + i*8;
						addDeltas(coeffs, refp, WIDTH/2);
					}
					coeffs += 8;
					input += 8;
				}
				coeffs += (WIDTH/2)*7;
				input  += (WIDTH/2)*7;
			}

			rlco = cfrm_current.rlc_data_lum;
			stat = *rlco;

			input = cfrm_current.lum_coeff;
			coeffs = chroma;
			uint8_t* lref = luminance_8bit;

			/* Only for timing purposes!!! */
			for (j = 0; j < HEIGHT/8; j++) {
				for (i = 0; i < WIDTH/8; i++) {
					ifwht(input, coeffs, WIDTH, WIDTH, (stat & PFRAME_BIT) ? 0 : 1);
					if (stat & PFRAME_BIT) {
						// add deltas
						uint8_t *refp = lref + j*8*WIDTH + i*8;
						addDeltas(coeffs, refp, WIDTH);
					}
					coeffs += 8;
					input += 8;
				}
				coeffs += (WIDTH)*7;
				input  += (WIDTH)*7;
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
		decodeFrame(&cfrm_current, chroma_8bit, luminance_8bit, chroma, luminance);
		diff2 = clock() - start2;

		int msec2 = diff2; // * 1000 / CLOCKS_PER_SEC;
		printf("read %d %d\n", cfrm_current.lum_sz, cfrm_current.chroma_sz);
		printf("%lf and %lf\n", (float)WIDTH*HEIGHT/cfrm_current.lum_sz * 100, (float)WIDTH*HEIGHT/2/cfrm_current.chroma_sz * 100);
		#ifdef WRITE_CRATIO
		fprintf(fp4, "%f, %f\n", (float)WIDTH*HEIGHT/cfrm_current.lum_sz * 100, (float)WIDTH*HEIGHT/2/cfrm_current.chroma_sz * 100);
		#endif

		#ifdef WRITE_TIMES
		fprintf(fp5, "%d, %d\n", msec, msec2);
		#endif
		#ifdef WRITE_PCOUNT
		fprintf(fp6,"%d,%d\n", pcount[0], pcount[1]);
		#endif

		int a;

		for (a = 0; a < WIDTH*HEIGHT; a++)
			luminance_8bit[a] = (uint8_t)luminance[a];
		//  int a;
		for (a = 0; a < (WIDTH*HEIGHT/2); a++)
			chroma_8bit[a] = (uint8_t)chroma[a];

			#ifdef WRITE_RAW
				writeRawFrame(fp3, luminance_8bit, chroma_8bit, WIDTH, HEIGHT);
			#endif

		SDL_Rect rect;

		SDL_LockYUVOverlay(bmp);
		memcpy(bmp->pixels[0], luminance_8bit, WIDTH*HEIGHT);
		memcpy(bmp->pixels[2], chroma_8bit, WIDTH*HEIGHT/4);
		memcpy(bmp->pixels[1], chroma_8bit + WIDTH*HEIGHT/4, WIDTH*HEIGHT/4);

		SDL_UnlockYUVOverlay(bmp);

		rect.x = 0;
		rect.y = 0;
		rect.w = WIDTH;
		rect.h = HEIGHT;
		SDL_DisplayYUVOverlay(bmp, &rect);

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
