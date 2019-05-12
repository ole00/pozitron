This demo generates a 15kHz video signal on GPIO port D.
The video signal is output as uzebox compatible RGB211+233 + Sync.
Uses frame buffer of 1 byte per pixel in resolution 240x224 @ 60 Hz.

Blue:  bits 7 - 6, 11 - 10 on GPIO Port D
Green: bits 5 - 3, 9 on GPIO Port D
Red:   bits 2 - 0, 8 on GPIO Port D
Sync:  bit 7 on GPIO Port B


A DAC made of resistors looks like this:


  o Sync                 o Blue - output to tv (Green and Red is the same as Blue, see above
  |                      |                      for pins)
  |                      |
  +----+       +----+----+----+----+
  |    |       |    |    |    |    |
 ---  ---     ---  ---  ---  ---  ---
 | |  | |     | |  | |  | |  | |  | |
 |1|  |3|     |1|  |2|  |4|  |1|  |3|
 |k|  |3|     |k|  |k|  |k|  |0|  |3|   <.... these are Resistors, metal film, 0.25W, 1% tolerance
 | |  |0|     | |  |2|  |7|  |k|  |0|
 | |  | |     | |  | |  | |  | |  | |
 ---  ---     ---  ---  ---  ---  ---
  |    |       |    |    |    |    |
  |    |       |    |    |    |    |
  o    V       o    o    o    o    V
 B7   GND     D7   D6   D11  D10  GND

