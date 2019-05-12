/*
 *  Uzebox Kernel - Mode 1
 *  Copyright (C) 2008  Alec Bourque
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Uzebox is a reserved trade mark
*/

#include <stdbool.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "uzebox.h"
#include "intro.h"

#include "stm_defines.h"
#include "uzeboxVideoEngineCore.h"

#if INTRO_LOGO !=0
	#if TILE_WIDTH == 6
		#define LOGO_X_POS 18
		#include "videoMode1/uzeboxlogo_6x8.pic.inc"
		#include "videoMode1/uzeboxlogo_6x8.map.inc"			
	#else
		#define LOGO_X_POS 13
		#include "videoMode1/uzeboxlogo_8x8.pic.inc"
		#include "videoMode1/uzeboxlogo_8x8.map.inc"
	#endif
#endif


// NTSC values
#define GFX_TOP_LINE 26
#define GFX_BOTTOM_LINE (GFX_TOP_LINE + 224)
#define MAX_W		256
#define MAX_H		224

extern uint8_t currentLine;

extern volatile uint32_t videoLine;
uint16_t* drawBuffer;

void blinkLed(void); 

uint8_t* vram[VRAM_SIZE];

uint8_t* tile_table = 0;
uint8_t* font_table = 0;

void SetTile(char x, char y, unsigned  int tileId) {
	vram[(y * VRAM_TILES_H) + x] = tile_table + tileId * (TILE_WIDTH * TILE_HEIGHT);
}

void SetFont(char x, char y, unsigned char  tileId) {
	vram[(y * VRAM_TILES_H) + x] =  font_table + tileId * (TILE_WIDTH * TILE_HEIGHT);
}

void ClearVram(void) {
	uint16_t i = 0;
	uint8_t* empty_tile = (tile_table == 0) ? font_table : tile_table;
	while(i < VRAM_SIZE) {
		vram[i++] = empty_tile; //pointer to tile index 0
	}
}

void SetTileTable(const char* data) {
	tile_table = (uint8_t*) data;
}

void SetFontTable(const char *data) {
	font_table = (uint8_t*) data;
}


//Callback invoked by UzeboxCore.Initialize()
void DisplayLogo(){

	#if INTRO_LOGO !=0
		InitMusicPlayer(logoInitPatches);
		SetTileTable(uzeboxlogo);
		SetFontTable(uzeboxlogo);
				
		//draw logo
		ClearVram();
		WaitVsync(15 * (MODE1_FAST_VSYNC+1));		

		#if INTRO_LOGO == 1 
			TriggerFx(0,0xff,true);
		#endif
		blinkLed();

		DrawMap(LOGO_X_POS,12,map_uzeboxlogo);
		WaitVsync(3);
		DrawMap(LOGO_X_POS,12,map_uzeboxlogo2);
		WaitVsync(2);
		DrawMap(LOGO_X_POS,12,map_uzeboxlogo);
		blinkLed();
		#if INTRO_LOGO == 2
			SetMasterVolume(0xc0);
			TriggerNote(3,0,16,0xff);
		#endif 
	
		WaitVsync(32 * (MODE1_FAST_VSYNC+1));
		ClearVram();
		WaitVsync(10 * (MODE1_FAST_VSYNC+1));
		blinkLed();
	#endif	
}


//Callback invoked by UzeboxCore.Initialize()
void InitializeVideoMode(){}

//Callback invoked during hsync
void VideoModeVsync() {		
	ProcessFading();
}

#pragma GCC push_options
#pragma GCC optimize ("O2")

#if TILE_WIDTH == 8
#error unsupported TILE_WIDTH 8 
#endif



#define PIX6_0 *(buff++) = *tileDataPtr++; tileIndex++; 
#define PIX6_1 *(buff++) = *tileDataPtr++; vramPtr = vram + tileIndex; 
#define PIX6_2 *(buff++) = *tileDataPtr++; tileDataNextPtr = *vramPtr; 
#define PIX6_3 *(buff++) = *tileDataPtr++; tileDataNextPtr += tileLine;
#define PIX6_4 *(buff++) = *tileDataPtr++; 
#define PIX6_5 *(buff++) = *tileDataPtr; tileDataPtr = tileDataNextPtr; 

// Color tile 6 pixels
#define TILE_6 PIX6_0 PIX6_1  PIX6_2 PIX6_3  PIX6_4  PIX6_5 
/* 
* Paint single line of this videomode
*/
void sub_video_mode1(void) {
	uint16_t* buff = drawBuffer + 8;
	uint16_t line = currentLine; 

	uint8_t  tileLine = (line % TILE_HEIGHT) * TILE_WIDTH; 
	uint16_t tileIndex = (line / TILE_HEIGHT) * VRAM_TILES_H;

	uint8_t* tileDataPtr = vram[tileIndex] + tileLine;
	uint8_t* tileDataNextPtr;
	uint8_t** vramPtr;

	TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 //48 pixels
	TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 //96 pixels
	TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 //144 pixels
	TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 //192 pixels
	TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 TILE_6 //240 pixels

}
