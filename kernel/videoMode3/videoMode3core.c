/*
 *  Uzebox Kernel - Mode 3
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
#include <string.h>

#include "videoMode3.h" 

#pragma GCC push_options
#pragma GCC optimize ("O2")

extern uint16_t* drawBuffer;
extern volatile uint16_t currentLine;

u8 vram[VRAM_SIZE];
struct SpriteStruct sprites[MAX_SPRITES];
uint8_t* tile_table = 0;
uint8_t ram_tiles[RAM_TILES_COUNT * TILE_HEIGHT * TILE_WIDTH];
struct BgRestoreStruct ram_tiles_restore[RAM_TILES_COUNT];
const char* sprites_tile_banks[4];
extern u8 free_tile_index;
extern u8 userRamTilesCount;

u8 fontTileIndex = 0;

void ClearVram(void) {
	uint16_t i = 0;
    while(i < VRAM_SIZE) {
		vram[i++] = RAM_TILES_COUNT;
	}
}

void SetFontTilesIndex(u8 index) {
    fontTileIndex = index;
}

void SetSpritesTileTable(const char* tileset) {
    sprites_tile_banks[0] = tileset;
    sprites_tile_banks[1] = tileset;
    sprites_tile_banks[2] = tileset;
    sprites_tile_banks[3] = tileset;  
}

void SetSpritesTileBank(u8 bankIndex, const char* tileData) {
    sprites_tile_banks[bankIndex] = tileData;
}

void SetTile(char x, char y, unsigned  int tileId) {
	vram[(y * VRAM_TILES_H) + x] = (u8) (tileId + RAM_TILES_COUNT) ;
}

u8 GetTile(u8 x, u8 y) {
    return vram[(y * VRAM_TILES_H) + x] - RAM_TILES_COUNT;
}

void SetFont(char x, char y, unsigned char  tileId) {
    vram[(y * VRAM_TILES_H) + x] = (u8) (tileId + RAM_TILES_COUNT + fontTileIndex);
}

void SetTileTable(const char* data) {
    tile_table = (uint8_t*) data;
}

/*Copy srcTile from the active tileset in flash to destTile ramtile*/
void CopyFlashTile(u8 srcTile, u8 dstTile) {
    u32 srcOffset = ((u32) srcTile) * TILE_HEIGHT * TILE_WIDTH;
    u32 dstOffset = ((u32) dstTile) * TILE_HEIGHT * TILE_WIDTH;
    memcpy(ram_tiles + dstOffset, tile_table + srcOffset, TILE_HEIGHT * TILE_WIDTH );
}

/*Copy srcTile ramtile to destTile ramtile*/
void CopyRamTile(u8 srcTile,u8 dstTile) {
    u32 srcOffset = ((u32) srcTile) * TILE_HEIGHT * TILE_WIDTH;
    u32 dstOffset = ((u32) dstTile) * TILE_HEIGHT * TILE_WIDTH;
    memcpy(ram_tiles + dstOffset, ram_tiles + srcOffset, TILE_HEIGHT * TILE_WIDTH );
}

#define PIX8_0 *(buff++) = *tileDataPtr++; tileIndex++; 
#define PIX8_1 *(buff++) = *tileDataPtr++; pixelOffset = vram[tileIndex];  
#define PIX8_2 *(buff++) = *tileDataPtr++; if (pixelOffset < RAM_TILES_COUNT) {tileDataNextPtr = ram_tiles;} \
                                             else  { tileDataNextPtr = tile_table; pixelOffset -= RAM_TILES_COUNT;}
#define PIX8_3 *(buff++) = *tileDataPtr++; pixelOffset *= TILE_HEIGHT * TILE_WIDTH;
#define PIX8_4 *(buff++) = *tileDataPtr++; pixelOffset += tileLine;
#define PIX8_5 *(buff++) = *tileDataPtr++; tileDataNextPtr += pixelOffset;
#define PIX8_6 *(buff++) = *tileDataPtr++; 
#define PIX8_7 *(buff++) = *tileDataPtr; tileDataPtr = tileDataNextPtr; 

// line of tile of 8 pixels
#define TILE_8 PIX8_0 PIX8_1 PIX8_2 PIX8_3 PIX8_4 PIX8_5 PIX8_6 PIX8_7


/* 
* Paint single line of this videomode
*/
void sub_video_mode3(void) {
    uint8_t i;
	uint16_t* buff = drawBuffer + 6;
	uint16_t line = currentLine; 

	uint8_t  tileLine = (line % TILE_HEIGHT) * TILE_WIDTH; //start of the tile line
	uint16_t tileIndex = (line / TILE_HEIGHT) * VRAM_TILES_H;

	uint8_t* tileDataPtr;  //pointer to the tile's line pixels 
    uint8_t* tileDataNextPtr; //pointer to the next tile's line pixels 

    // set up sprite tiles
    for (i = userRamTilesCount; i < free_tile_index; i++){
        ram_tiles_restore[i].tileIndex = *ram_tiles_restore[i].addr;
        *ram_tiles_restore[i].addr = i;
    }

    // set-up variables for the first tile
    uint32_t pixelOffset = vram[tileIndex]; // pixelOffset is offset/address of the left-most pixel of the tile and its current line 
    // RAM tiles start with idices 0 to (RAM_TILES_COUNT - 1)
    if (pixelOffset < RAM_TILES_COUNT) {
        tileDataPtr = ram_tiles; //use RAM tile buffer
    } else
    // ROM tiles start with indices RAM_TILES_COUNT to (RAM_TILES_COUNT + ROM_TILES_COUNT - 1)
    {
        tileDataPtr = tile_table; // use ROM tile buffer
        pixelOffset -= RAM_TILES_COUNT; //ensure ROM tile index starts from index 0
    }
    pixelOffset *= (TILE_HEIGHT * TILE_WIDTH); // now the offset points to the top-left pixel of the tile
    pixelOffset += tileLine;  // now the offset points to the tile's line first pixel
    tileDataPtr += pixelOffset;

    TILE_8 TILE_8 TILE_8 TILE_8 TILE_8 TILE_8 //48 pixels
    TILE_8 TILE_8 TILE_8 TILE_8 TILE_8 TILE_8 //96 pixels
	TILE_8 TILE_8 TILE_8 TILE_8 TILE_8 TILE_8 //144 pixels
	TILE_8 TILE_8 TILE_8 TILE_8 TILE_8 TILE_8 //192 pixels
	TILE_8 TILE_8 TILE_8 TILE_8 TILE_8 TILE_8 //240 pixels

    // restore sprite background
    RestoreBackground();
}