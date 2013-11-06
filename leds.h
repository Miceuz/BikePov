#ifndef LEDS
#define LEDS

#include "util.h"
#define SP_PORT PORTC
#define SP_DATA PC2
#define SP_LE PC1
#define SP_SH_CLOCK PC3

void sendToLeds(uint32_t);
void initLeds();
void blink(word ms);

#endif
