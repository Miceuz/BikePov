/* Name: main.c
 * Project: custom-class, a basic USB example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-09
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: main.c 790 2010-05-30 21:00:26Z cs $
 */

/*
This example should run on most AVRs with only little changes. No special
hardware resources except INT0 are used. You may have to change usbconfig.h for
different I/O pins for USB. Please note that USB D+ must be the INT0 pin, or
at least be connected to INT0 as well.
We assume that an LED is connected to port B bit 0. If you connect it to a
different port or bit, change the macros below:
*/
#include <avr/io.h>
#include <avr/interrupt.h>  /* for sei() */
#include <avr/wdt.h>
#include <avr/sleep.h>

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "usb.h"
#include "util.h"
#include "leds.h"
#include "eeprom.h"


uint16_t sensorTimer = 0;

void resetSensorTimer() {
	sensorTimer = 0;
	TCNT0 = 0;
}


void initRefreshRateTimer() {
	//start timer1 with no prescaler
	TCCR1B |= _BV(CS10);
	OCR1A = 0xffff/10;
	TIMSK |= _BV(OCIE1A);
}

void stopRefreshRateTimer() {
	TCCR1B &= ~_BV(CS10);
	TIMSK &= ~_BV(OCIE1A);
}

void startRefreshRateTimer() {
	//start timer with no prescaler
	TCCR1B |= _BV(CS10) | _BV(WGM12);
	//enable compare match A interrupt
	TIMSK |= _BV(OCIE1A);
}

void updateRefreshRate() {
	//set the Timer1 TOP value to 256 * sensorTimer + current Timer0 value, 
	//so we know exact processor clock tick count between two sensor triggers
	//as timer0 works on 256 prescaler and we have 256 radial pixels, there is 
	//no need to divide sensor time
	OCR1A = ((sensorTimer << 8) | TCNT0);
	startRefreshRateTimer();
}

void stopSensorTimer() {
	TCCR0 &= ~_BV(CS02);
	TIMSK &= ~_BV(TOIE0);
    
//    SENSOR_MEAT_PORT &=~ _BV(SENSOR_MEAT);
//    EEPROM_MEAT_PORT &=~ _BV(EEPROM_MEAT);
}

void startSensorTimer() {
	TCCR0 |= _BV(CS02);
	TIMSK |= _BV(TOIE0);
    
//    SENSOR_MEAT_PORT |= _BV(SENSOR_MEAT);
//    EEPROM_MEAT_PORT |= _BV(EEPROM_MEAT);
}

uint16_t currPictureAddress = 0;
uint8_t currPicture = 0;
uint16_t pictureSize = 1024;
uint16_t currLineAddress = 0;

uint16_t rotationOffset = 0;
uint8_t pictureDelay = 0;
uint8_t pictureCount = 1;
uint8_t pictureDelayCounter = 0;

void nextPicture() {
    pictureDelayCounter = 0;
    currPicture++;
    if(currPicture >= pictureCount) {
        currPicture = 0;
    }
    currPictureAddress = currPicture * pictureSize;
}

void resetDisplayLine() {
    if(pictureDelay > 0){
        if(pictureDelayCounter++ >= pictureDelay - 1) {
            nextPicture();
        }
    }
	currLineAddress = currPictureAddress + rotationOffset;
}

void resetPicture() {
    currPicture = 0;
    currPictureAddress = currPicture * pictureSize;
    currLineAddress = currPictureAddress + rotationOffset;
    pictureDelayCounter = 0;
}

unsigned char isSleeping = 0;


//sensor trigger interrupt gets called when sensor is triggered
SIGNAL(INT1_vect) {
	//if sensor time is small enough to fit into compare match register
    if(isSleeping) {
        isSleeping = 0;
        startSensorTimer();
    }

    if(sensorTimer < 0xFF) {
        resetDisplayLine();
        updateRefreshRate();
    } else {
        //sensor time is too long, don't try to display 
        stopRefreshRateTimer();
        sendToLeds(0x80000002);
    }
	resetSensorTimer();
}

#define STANDBY_TIMEOUT 8

uint8_t isStandbyTimeout() {
	return (uint32_t) sensorTimer > ((uint32_t)F_CPU /256/256) * STANDBY_TIMEOUT;
}

//sensor trigger counter
SIGNAL(TIMER0_OVF_vect) {
	if(!isStandbyTimeout() && !isSleeping){
        sensorTimer++;
    } else {
		sendToLeds(0x00000000);
        stopRefreshRateTimer();
        stopSensorTimer();

        set_sleep_mode(SLEEP_MODE_IDLE);
        isSleeping = 1;
        resetPicture();
        sei();
        sleep_mode();
	}
}

//refresh rate timer
SIGNAL(TIMER1_COMPA_vect) {
    sei(); //enable nested interrupts, because we will need TWI interrupt to talk to EEPROM
    sendToLeds(readFromEeprom(currLineAddress));
    currLineAddress += 4;
    if((currLineAddress - currPictureAddress) >= pictureSize) {
        currLineAddress = currPictureAddress;
    }
}



int __attribute__((noreturn)) main(void)
{
	wdt_disable();
	initUsb();
	initButton();
	initLeds();
	initSensor();
	initEeprom();	
	initRefreshRateTimer();
    
    ACSR |= _BV(ACD);//disable analog comparator
    
    
    rotationOffset = getRotationOffset();
    pictureDelay = getPictureDelay();
    pictureCount = getPictureCount();

	sei();

	uint32_t a = 1;
	byte direction = 0xff;
	//sendToLeds(0x80000001);
	_delay_ms(1000);
	
    for(;;){
        usbPoll();
	/*	
		sendToLeds(a);
		_delay_ms(7);
		
		if(direction) {
			a = (a << 1); 
		} else {
			a = (a >> 1);
		}
		if (a == 0x80000000 || a == 1) {
			direction = ~ direction;
		}
*/
        uint16_t buttonCounter=0;
        uint8_t buttonPress = 0;
        while(buttonPressed()){
            uint32_t t = 512;
            while(t-- > 0){
                //NOTHING
            }
            if(buttonCounter++ >= 255*255){
                goToReset();
            }
            buttonPress=1;
        }
        if(buttonPress) {
            nextPicture();
        }
  }
}

/* ------------------------------------------------------------------------- */
