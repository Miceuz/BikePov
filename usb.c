#include "usb.h"
#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include <avr/wdt.h>
#include "usbdrv.h"
#include "leds.h"
#include "eeprom.h"

#include "requests.h"       /* The custom request numbers we use */

uint16_t currEepromAddress = 0x00;

void initUsb(){
	
	usbInit();
	usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
	uchar   i;
	i = 0;
	while(--i){             /* fake USB disconnect for > 250 ms */
		wdt_reset();
		_delay_ms(1);
	}
	usbDeviceConnect();
}

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (void *)data;
	static uchar    dataBuffer[4];  /* buffer must stay valid when usbFunctionSetup returns */
	
    if(rq->bRequest == CUSTOM_RQ_ECHO){ /* echo -- used for reliability tests */
        dataBuffer[0] = rq->wValue.bytes[0];
        dataBuffer[1] = rq->wValue.bytes[1];
        dataBuffer[2] = rq->wIndex.bytes[0];
        dataBuffer[3] = rq->wIndex.bytes[1];
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 4;
    }else if(rq->bRequest == CUSTOM_RQ_GET_STATUS){
        dataBuffer[0] = 0;//((LED_PORT_OUTPUT & _BV(LED_BIT)) != 0);
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 1; /* tell the driver to send 1 byte */
    } else if(rq->bRequest == SET_ROTATION_OFFSET) {
        uint16_t rotationOffset = ((uint16_t)rq->wValue.bytes[1]) << 8 | (uint16_t) rq->wValue.bytes[0];
        setRotationOffset(rotationOffset);
        
        return 0;
    } else if(rq->bRequest == SET_PICTURE_DELAY) {
        setPictureDelay(rq->wValue.bytes[0]);
        
        return 0;
    } else if(rq->bRequest == SET_PICTURE_COUNT) {
        setPictureCount(rq->wValue.bytes[0]);
        
        return 0;
    } else if(rq->bRequest == SAVE_TO_EEPROM) {
        sendToLeds(0);
		currEepromAddress = ((uint16_t)rq->wValue.bytes[1]) << 8 | (uint16_t) rq->wValue.bytes[0];
		return USB_NO_MSG;
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}


uchar usbFunctionWrite(uchar *data, uchar len) {
	uint32_t leds;
	leds = ((uint32_t) data[0]) << 24 | 
	((uint32_t) data[1]) << 16 |
	((uint32_t) data[2]) << 8 |
	((uint32_t) data[3]);

	//sendToLeds(currEepromAddress);
	writeToEeprom(currEepromAddress, leds);
    return 1;             // return 1 if we have all data
}
