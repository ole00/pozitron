
/* Includes */
#include <stddef.h>
#include "stm32f10x.h"


#define NOP __asm("nop")


// NTSC values
#define MAX_W		256
#define MAX_H		224


/*
Colors are set to be backward compatible with Uzebox (BGR233),
but still allowing 12bit graphics (RGB444) for new video modes.
That is the lower byte of the GPIO video port outputs RGB332
and the high byte of GPIO video port outputs the color complement
as RGB112. When wired correctly the DAC can produce either RGB332
or RGB444 colors depending whether the high byte of the GPIO port
is used.

 ------------------------------------------------------------------------------------
   High byte                               ||    Low byte
 ------------------------------------------++----------------------------------------
    15  | 14 | 13 | 12 | 11 | 10 |  9 |  8 ||  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
 ------------------------------------------++----------------------------------------
    x   | x  | x  | x  | B1 | B0 | G0 | R0 || B3 | B2 | G3 | G2 | G1 | R3 | R2 | R1 |
  
Sync - composite SYNC (negative)
x - not used, but can be used to upgrade to RGB555 if needed
R3-0: red bits
G3-0: green bits
B3-0: blue bits

Pros:
+ compatibility with uzebox
+ if high byte DAC is not wired up the colors will still look OK-ish.
+ more colors and proper shades of gray
Cons:
- programtic manipulation of colours is more CPU intensive (more calculations),
  but can be mittigated by using precomputed colors in the palette
*/

//when color is set make sure both sync and rgb pins are on
// converts the more natural RGB444 into uzebox compatible RGB112+332 (12 bit color)
// the foolowing macro: first line is calulation of uzebox's BGR233
//                      the second line is the complementary colors BGR211
#define GET_COLOR(x) (  (((x) & 0xE00) >> 9) | (((x) & 0xE0) >> 2) | (((x) & 0xC) << 4 ) \
			                     | (((x) & 0x100) )     | (((x) & 0x10) << 5) | (((x) & 0x3) << 10 ))

//gray color
#define VOUT_GRAY				GET_COLOR(0x444)

//bright gray color
#define VOUT_LIGHT_GRAY  		GET_COLOR(0x888)

#define OUT_1		0
#define OUT_2	    VOUT_GRAY
#define OUT_3		VOUT_LIGHT_GRAY


//8 x 8 tiles
static uint8_t tiles[][64] = {
		//digit 0
		{
				2, 3, 3, 3, 3, 3, 2, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				2, 3, 3, 3, 3, 3, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2,
		},
		//digit 1
		{
				2, 2, 2, 2, 3, 2, 2, 2,
				2, 2, 2, 3, 3, 2, 2, 2,
				2, 2, 3, 3, 3, 2, 2, 2,
				2, 2, 2, 3, 3, 2, 2, 2,
				2, 2, 2, 3, 3, 2, 2, 2,
				2, 2, 2, 3, 3, 2, 2, 2,
				2, 2, 3, 3, 3, 3, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2,
		},
		//digit 2
		{
				2, 3, 3, 3, 3, 3, 2, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				2, 2, 2, 2, 2, 3, 3, 2,
				2, 3, 3, 3, 3, 3, 2, 2,
				3, 3, 2, 2, 2, 2, 2, 2,
				3, 3, 2, 2, 2, 2, 2, 2,
				3, 3, 3, 3, 3, 3, 3, 2,
				2, 2, 2, 2, 2, 2, 2, 2,
		},
		//digit 3
		{
				2, 3, 3, 3, 3, 3, 2, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				2, 2, 2, 2, 2, 3, 3, 2,
				2, 2, 2, 3, 3, 3, 2, 2,
				2, 2, 2, 2, 2, 3, 3, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				2, 3, 3, 3, 3, 3, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2,
		},
		//digit 4
		{
				2, 2, 2, 3, 3, 3, 3, 2,
				2, 2, 3, 3, 2, 3, 3, 2,
				2, 3, 3, 2, 2, 3, 3, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				3, 3, 3, 3, 3, 3, 3, 2,
				2, 2, 2, 2, 2, 3, 3, 2,
				2, 2, 2, 2, 2, 3, 3, 2,
				2, 2, 2, 2, 2, 2, 2, 2,
		},
		//digit 5
		{
				3, 3, 3, 3, 3, 3, 3, 2,
				3, 3, 2, 2, 2, 2, 2, 2,
				3, 3, 2, 2, 2, 2, 2, 2,
				2, 3, 3, 3, 3, 3, 2, 2,
				2, 2, 2, 2, 2, 3, 3, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				2, 3, 3, 3, 3, 3, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2,
		},
		//digit 6
		{
				2, 3, 3, 3, 3, 3, 2, 2,
				3, 3, 2, 2, 2, 2, 2, 2,
				3, 3, 2, 2, 2, 2, 2, 2,
				3, 3, 3, 3, 3, 3, 2, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				2, 3, 3, 3, 3, 3, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2,
		},
		//digit 7
		{
				3, 3, 3, 3, 3, 3, 3, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				2, 2, 2, 2, 3, 3, 2, 2,
				2, 2, 2, 3, 3, 2, 2, 2,
				2, 2, 2, 3, 3, 2, 2, 2,
				2, 2, 2, 3, 3, 2, 2, 2,
				2, 2, 2, 3, 3, 2, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2,
		},
		//digit 8
		{
				2, 3, 3, 3, 3, 3, 2, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				2, 3, 3, 3, 3, 3, 2, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				2, 3, 3, 3, 3, 3, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2,
		},
		//digit 9
		{
				2, 3, 3, 3, 3, 3, 2, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				3, 3, 2, 2, 2, 3, 3, 2,
				2, 3, 3, 3, 3, 3, 3, 2,
				2, 2, 2, 2, 2, 3, 3, 2,
				2, 2, 2, 2, 2, 3, 3, 2,
				2, 3, 3, 3, 3, 3, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2,
		},
};


static uint8_t frameBuff[MAX_W * MAX_H];
static uint16_t palette[256];

uint8_t* framePtr = frameBuff;


extern volatile uint32_t videoLine;
extern volatile uint32_t videoFrame;
uint16_t* drawBuffer;

void initVideo();
static void drawNumber(uint16_t x, uint16_t y, uint32_t value, uint16_t positions);



void drawScanLine(void) {
		uint16_t posX = 0;
        uint16_t* buff = drawBuffer + 4;
        uint8_t* ptr = framePtr + (MAX_W * videoLine);
		
		posX = 0;
        

		//paint 240 pixels on this line
		while (posX < 240) {
			//output 8 pixels 
			*(buff++) = palette[ptr[posX++]];
			*(buff++) = palette[ptr[posX++]];
			*(buff++) = palette[ptr[posX++]];
			*(buff++) = palette[ptr[posX++]];
			*(buff++) = palette[ptr[posX++]];
			*(buff++) = palette[ptr[posX++]];
			*(buff++) = palette[ptr[posX++]];
			*(buff++) = palette[ptr[posX++]];
		}

}


static void paintTile(uint16_t x, uint16_t y, uint16_t tileIndex) {
	uint16_t j, i;
	uint8_t* ptr = framePtr;
	uint8_t* src = tiles[tileIndex];

	framePtr = &frameBuff[MAX_W * y  + x];

	for (j = 0; j < 8; j++) {
			for (i = 0; i < 8; i++) {
				framePtr[i] = src[i];
			}
			src += 8;
			framePtr += MAX_W;
	}
	framePtr = ptr;
}

static void drawNumber(uint16_t x, uint16_t y, uint32_t value, uint16_t positions) {
	uint16_t i;
	uint16_t posX = x + ((positions - 1) * 8);

	for (i = 0 ; i < positions; i++) {
		paintTile(posX, y, (uint16_t)(value % 10));
		value /= 10;
		posX -= 8;
	}

}

static void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color) {
	uint16_t j, i;
	uint8_t* ptr = framePtr;

	framePtr = &frameBuff[MAX_W * y  + x];

	for (j = 0; j < h; j++) {
			for (i = 0; i < w; i++) {
				framePtr[i] = color;
			}
			framePtr += MAX_W;
	}
	framePtr = ptr;
}

static void initFrameBuffer(void) {
	int j, i;

	palette[0] = 0;
	palette[1] = OUT_1;
	palette[2] = OUT_2;
	palette[3] = OUT_3;

	//generate Blue Green Red and White color shades in the palette
	//use 16 gradients
	for (i = 0; i < 16; i++) {
		palette[16 + i] = GET_COLOR(i);                            //B
		palette[32 + i] = GET_COLOR(i << 4);                       //G
		palette[48 + i] = GET_COLOR(i << 8);                       //R
		palette[64 + i] = GET_COLOR(i | (i << 4) | (i << 8));      //Gray/White
	}

	//generate grid
	for (j = 0; j < MAX_H; j++) {
		framePtr = &frameBuff[MAX_W * j];
		if ((j & 0x7) == 0) {
			//solid bright line
			for (i = 0; i < MAX_W; i++) {
				framePtr[i] = 3;
			}
		} else {
			//grid line
			for (i = 0; i < MAX_W; i++) {
				framePtr[i] = (i & 0x7) == 0 ? 3 : 2;
			}
		}
	}
	framePtr = frameBuff;

	// gray rectangle under the frame counter
	fillRect(12, 4, 90, 16, 2);

	for (i = 0; i <= 9; i++) {
		paintTile(16  ,8, i);
	}

	drawNumber(16, 8, videoFrame, 8);

	//generate R G B  shade pattern / color bars
	for (i = 0; i < 16; i++) {
		fillRect(56,      48 + (i*8), 32, 8, 48 + i ); //R
		fillRect(56 + 32, 48 + (i*8), 32, 8, 32 + i ); //G
		fillRect(56 + 64, 48 + (i*8), 32, 8, 16 + i ); //B
		fillRect(56 + 96, 48 + (i*8), 32, 8, 64 + i ); //W
	}

}

void Delay_ticks( uint32_t ticks )
{
  uint32_t i;
  ticks >>= 1;
  for(i = 0; i < ticks; i++)
  {
  	NOP;
  }
}

int main(void)
{
#ifdef SHOW_FRAME_COUNTER
	uint32_t frame = 0;
#endif /* SHOW_FRAME_COUNTER */

	initFrameBuffer();

    initVideo();

    //while(1); NOP;
#if 1

	while (1) {
#ifdef SHOW_FRAME_COUNTER
			if (frame != videoFrame) {
			    drawNumber(16, 8, videoFrame, 8);
				frame = videoFrame;
			}
		
#endif /* SHOW_FRAME_COUNTER */
	}
#endif
}

