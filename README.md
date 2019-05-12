# pozitron
port of uzebox game console to STM32F103VET6

This is Work-In-Progress in early stages.


Fetaures:
* 96 MHz CPU, 64 kbytes RAM, 512 kbytes flash
* 12 bit video output compatible with uzebox game console (BRG211+233)
* DMA gnerated video signal (inspired by bitbox game console)
* code repository structure similar to uzebox
* the aim is to be be source-code-compatible with uzebox code (for ported videomodes)

Requires:
* dev board from ebay or aliexpress (search for STM32F103VET6, see the dev board image
  in the board directory to find a matching one, other boards based on the same chip
  will most likely work as well)
* bunch of resitors an buttons wired according to the schematics 
  (see board/pzitron_schem.png). An IO board is planned so that the dev board can be
  plugged-in to it. The IO board will have the supporting circuitry (resistor DAC etc.)
  and connectors for joystick, audio out and power. 
* STlink USB dongle connected to the dev board to be able to upload binaries

What works:
* video mode1 240x224 / 60Hz, 6pixels per tile
* audio mixer and audio out on Port A4
* crude joystick controls via buttons wired on port E
* vmode1 demos in demos directory (DrMario, Maze, Megatris, MusicDemo, tutorial)
* the existing demos can be build and uploaded to the dev board via STlink usb dongle 

Not implemented yet:
* video mode 3 and others :), though vmode 3 is probably the next to come.
* sd card access
* custom bootloader and booting from sd card

How to build & run:
* enter demos/tutorial/default directory
* type: 'make clean all' to build
* type: 'sudo make burn' to upload to dev board
* press the reset button on the dev board to start the demo



