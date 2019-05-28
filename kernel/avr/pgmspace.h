#ifndef _PGMSPACE_H_
#define _PGMSPACE_H_

#include "defines.h"

uint16_t pgm_read_word(const void* addr);
#define pgm_read_byte(X) *(X)

#define PSTR(X) (const char*) (X)

#endif
