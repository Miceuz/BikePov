#include "eeprom.h"
#include "util.h"
#include "TWI_Master.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include "leds.h"
#include <avr/eeprom.h>
unsigned char messageBuf[MESSAGEBUF_SIZE];

void initEeprom() {
	//enable TWI channel pullups
	DDRC &= ~_BV(PC5);
	DDRC &= ~_BV(PC4);
	PORTC |= _BV(PC5) | _BV(PC4);
	TWI_Master_Initialise();
    
    EEPROM_MEAT_DDR |= _BV(EEPROM_MEAT);
    EEPROM_MEAT_PORT |= _BV(EEPROM_MEAT);
}

void TWI_Act_On_Failure_In_Last_Transmission ( unsigned char TWIerrorMsg )
{
	// A failure has occurred, use TWIerrorMsg to determine the nature of the failure
	// and take appropriate actions.
	// See header file for a list of possible failure messages.
	
	// Error codes are just the state codes defined in TWI_Master.h. These tend to be
	//	rather large to decode from a blinking LED. Since there are really only a few
	//	that we care about, use a SWITCH/CASE to put out easy codes.
	// Note that Receive Data NACK from Slave is expected so is not an error.
	switch (TWIerrorMsg & 0xf8)
	{
		case TWI_MTX_ADR_NACK:		// No Transmit Address Ack from Slave
			sendToLeds(1);
			break;
		case TWI_MTX_DATA_NACK:		// No Transmit Data Ack from Slave
			sendToLeds(2);
			break;		
		case TWI_MRX_ADR_NACK:		// No Receive Address Ack from Slave
			sendToLeds(4);
			break;		
		case TWI_ARB_LOST:			// Arbitration Lost -- How?
			sendToLeds(8);
			break;
		case TWI_NO_STATE:			// No State -- What happened?
			sendToLeds(16);
			break;
		case TWI_BUS_ERROR:			// Bus Error
			sendToLeds(32);
			break;
		default:					// Anything Else - further decoding is possible
			sendToLeds(64);
            break;
	}
}


void writeToEeprom(word address, uint32_t data) {
	messageBuf[0] = CHIP_ADDRESS  | (FALSE<<TWI_READ_BIT);
	messageBuf[1] = (address & 0xFF00) >> 8 ;       // Starting address in memory
	messageBuf[2] = (address & 0x00FF);
	messageBuf[3] = (data & 0xFF000000) >> 24;
	messageBuf[4] = (data & 0x00FF0000) >> 16;
	messageBuf[5] = (data & 0x0000FF00) >> 8;
	messageBuf[6] = (data & 0x000000FF);
	
	TWI_Start_Read_Write( messageBuf, 7);
	
	while ( TWI_Transceiver_Busy() ) {
//		wdt_reset();
	}
	
	if (TWI_statusReg.lastTransOK ){
		//blink(2000);
		//blink(2000);
	} else {
		byte i = 5;
		while(i-- > 0){
			//blink(50);
		}
		TWI_Act_On_Failure_In_Last_Transmission( TWI_Get_State_Info( ) );
	}

	//int i;
	//for(i = 0; i < MESSAGEBUF_SIZE; i++ ) {
	//	messageBuf[i] = 0;
	//}	
}

uint32_t readFromEeprom(word address) {
	messageBuf[0] = CHIP_ADDRESS | (TRUE<<TWI_READ_BIT);
	messageBuf[1] = (address & 0xFF00) >> 8 ;       // Starting address in memory
	messageBuf[2] = (address & 0x00FF);

	TWI_Start_Random_Read( messageBuf, 5);
	while ( TWI_Transceiver_Busy() ){
		wdt_reset();
	}
	
	if (!TWI_statusReg.lastTransOK ){
		byte i = 5;
		while(i-- > 0){
			//blink(70);
		}
		TWI_Act_On_Failure_In_Last_Transmission( TWI_Get_State_Info( ) );
	} else {
		TWI_Read_Data_From_Buffer( messageBuf, 5 );
		
		uint32_t ret;
		ret = ((uint32_t) messageBuf[1]) << 24 | 
			  ((uint32_t) messageBuf[2]) << 16 |
			  ((uint32_t) messageBuf[3]) << 8 |
			  ((uint32_t) messageBuf[4]);

		return ret;
	}
	return 0;
}

#define ROTATION_OFFSET_ADDRESS 0x00
#define PICTURE_DELAY_ADDRESS 0x03
#define PICTURE_COUNT_ADDRESS 0x04

extern uint16_t rotationOffset;
extern uint8_t pictureDelay;
extern uint8_t pictureCount;

void setRotationOffset(uint16_t rotOffset) {
    eeprom_write_word(ROTATION_OFFSET_ADDRESS, rotOffset);
    rotationOffset = rotOffset;
}

uint16_t getRotationOffset() {
    return eeprom_read_word(ROTATION_OFFSET_ADDRESS);
}

void setPictureDelay(uint8_t d) {
    eeprom_write_byte(PICTURE_DELAY_ADDRESS, d);
    pictureDelay = d;
}

uint8_t getPictureDelay() {
    return eeprom_read_byte(PICTURE_DELAY_ADDRESS);
}

void setPictureCount(uint8_t c) {
    eeprom_write_byte(PICTURE_COUNT_ADDRESS, c);
    pictureCount = c;
}

uint8_t getPictureCount() {
    return eeprom_read_byte(PICTURE_COUNT_ADDRESS);
}
