#include<avr/io.h>
#include "leds.h"
#include "util.h"

void sendToLeds(uint32_t b){
	 uint32_t mask = 0x80000000;
	
	while(mask > 0) {
		SP_PORT &=~ _BV(SP_SH_CLOCK);
		if(b & mask) {
			SP_PORT |= _BV(SP_DATA);
		} else {
			SP_PORT &=~ _BV(SP_DATA);
		}
		SP_PORT |= _BV(SP_SH_CLOCK);
		mask = mask >> 1;
	}
	SP_PORT &=~ _BV(SP_SH_CLOCK);
	SP_PORT |= _BV(SP_LE);
	SP_PORT &=~ _BV(SP_LE);
	 
}

//enable outouts to led latches
inline void initLeds() {
	DDRC |= _BV(SP_LE) | _BV(SP_DATA) | _BV(SP_SH_CLOCK);
}

void blink(word ms) {
	sendToLeds(1);
	_delay_ms(ms);
	sendToLeds(0);
	_delay_ms(ms);
}

