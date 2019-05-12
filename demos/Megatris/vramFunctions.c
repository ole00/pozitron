#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <uzebox.h>

const uint16_t* tileMap; //inital content of the vram

extern uint8_t* tile_table; // the tile pixels
extern uint8_t* vram[VRAM_SIZE]; // each item in vram is a pointer to the 1st tile pixel in the tile_table

void RestoreTile(char x,char y) {
    uint16_t index = (y * VRAM_TILES_H) + x;
    vram[index] = tile_table +  (tileMap[index] * (TILE_WIDTH * TILE_HEIGHT));
}

void LoadMap(void) {
    uint16_t i;

    for (i = 0; i < VRAM_SIZE; i++ ) {
        vram[i] = tile_table +  (tileMap[i] * (TILE_WIDTH * TILE_HEIGHT));
    }
    
}

void SetTileMap(const uint16_t* data) {
	tileMap = data;
}
