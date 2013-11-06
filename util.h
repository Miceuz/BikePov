#ifndef UTIL
#define UTIL

#define byte unsigned char
#define word unsigned int

#define BUTTON_BIT PB1
#define BUTTON_PORT_OUTPUT PORTB
#define BUTTON_PORT_INPUT PINB
#define BUTTON_PORT_DDR DDRB

#define SENSOR_MEAT PB0
#define SENSOR_MEAT_DDR DDRB
#define SENSOR_MEAT_PORT PORTB


byte buttonPressed();
void initButton();
void initSensor();
void goToReset();
void _delay_ms(uint16_t __ms);

#endif
