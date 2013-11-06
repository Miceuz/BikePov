#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include "util.h"
#include "leds.h"


byte buttonPressed() {
	return !(BUTTON_PORT_INPUT & _BV(BUTTON_BIT));
}

void initButton() {
	BUTTON_PORT_DDR &=~ _BV(BUTTON_BIT);
	BUTTON_PORT_OUTPUT |= _BV(BUTTON_BIT);//pullup for button
}

void initSensorTimer() {
	//start timer0 with 256 prescaler
	TCCR0 |= _BV(CS02);
	//Timer0 Overflow Interrupt Enable 
	TIMSK |= _BV(TOIE0);
}

void initSensor() {
	//enable INT1 interrupt
	MCUCR |= _BV(ISC11);//on falling edge
	GIMSK |= _BV(INT1);
	
    SENSOR_MEAT_DDR |= _BV(SENSOR_MEAT);
    SENSOR_MEAT_PORT |= _BV(SENSOR_MEAT);
    
	initSensorTimer();
}	

void goToReset() {
	cli();
	sendToLeds(0);
	wdt_enable(WDTO_1S);
	while(1){
		sendToLeds(0x80000000);
		_delay_ms(10);
		sendToLeds(0);
		_delay_ms(40);			
	}
}


void _delay_ms(uint16_t __ms) {
	unsigned int x;

	while(__ms--){
		x=5600;       //empirically determined fudge factor 16mhz
		while(x--);
	}
}

