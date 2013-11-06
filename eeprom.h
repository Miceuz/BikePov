#ifndef EEPROM
#define EEPROM 1

#include<avr/io.h>
#include "util.h"

#define MESSAGEBUF_SIZE 7
#define CHIP_ADDRESS  0b10100000
#define EEPROM_MEAT PC0
#define EEPROM_MEAT_DDR DDRC
#define EEPROM_MEAT_PORT PORTC


void initEeprom();
void writeToEeprom(word address, uint32_t data);
uint32_t readFromEeprom(word address);

void setRotationOffset(uint16_t rotOffset);
uint16_t getRotationOffset();
void setPictureDelay(uint8_t d);
uint8_t getPictureDelay();
void setPictureCount(uint8_t c);
uint8_t getPictureCount();

#endif
