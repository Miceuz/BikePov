/* Name: set-led.c
 * Project: custom-class, a basic USB example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-10
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: set-led.c 692 2008-11-07 15:07:40Z cs $
 */

/*
General Description:
This is the host-side driver for the custom-class example device. It searches
the USB for the LEDControl device and sends the requests understood by this
device.
This program must be linked with libusb on Unix and libusb-win32 on Windows.
See http://libusb.sourceforge.net/ or http://libusb-win32.sourceforge.net/
respectively.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>        /* this is libusb */
#include "opendevice.h" /* common code moved to separate module */

#include "../requests.h"   /* custom request numbers */
#include "../usbconfig.h"  /* device's VID/PID and names */

static void usage(char *name)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s eeprom filename ....... upload picture from filename to spoke\n", name);
    fprintf(stderr, "  %s rotationOffset word ....... set rotation offset to 16bit unsigned integer \n", name);
    fprintf(stderr, "  %s pictureDelay byte ....... set slideDelay to 8bit unsigned integer -- how many wheel rotations should one slide be displayed \n", name);
    fprintf(stderr, "  %s pictureCount byte ....... set pictureCount to 8bit unsigned integer -- how many pictures total are there in spoke\n", name);
}

int main(int argc, char **argv)
{
usb_dev_handle      *handle = NULL;
const unsigned char rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
char                vendor[] = {USB_CFG_VENDOR_NAME, 0}, product[] = {USB_CFG_DEVICE_NAME, 0};
char                buffer[4];
int                 cnt, vid, pid, isOn;

    usb_init();
    if(argc < 3){   /* we need at least one argument */
        usage(argv[0]);
        exit(1);
    }
    /* compute VID/PID from usbconfig.h so that there is a central source of information */
    vid = rawVid[1] * 256 + rawVid[0];
    pid = rawPid[1] * 256 + rawPid[0];
    /* The following function is in opendevice.c: */
    if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
        fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
        exit(1);
    }
    /* Since we use only control endpoint 0, we don't need to choose a
     * configuration and interface. Reading device descriptor and setting a
     * configuration and interface is done through endpoint 0 after all.
     * However, newer versions of Linux require that we claim an interface
     * even for endpoint 0. Enable the following code if your operating system
     * needs it: */
#if 0
    int retries = 1, usbConfiguration = 1, usbInterface = 0;
    if(usb_set_configuration(handle, usbConfiguration) && showWarnings){
        fprintf(stderr, "Warning: could not set configuration: %s\n", usb_strerror());
    }
    /* now try to claim the interface and detach the kernel HID driver on
     * Linux and other operating systems which support the call. */
    while((len = usb_claim_interface(handle, usbInterface)) != 0 && retries-- > 0){
#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
        if(usb_detach_kernel_driver_np(handle, 0) < 0 && showWarnings){
            fprintf(stderr, "Warning: could not detach kernel driver: %s\n", usb_strerror());
        }
#endif
    }
#endif

	if(strcasecmp(argv[1], "eeprom") == 0){
        
        FILE *f;
         char buffer[1024];
        int n;
        
        f = fopen(argv[2], "rb");
        if (f) {
            n = fread(buffer, 1024, 1, f);
        } else {
            fprintf(stderr, "Could not open file '%s'", argv[2]);
            usage(argv[0]);
        }
        unsigned char bank = atoi(argv[3]);
        
        unsigned int startAddress = bank * 1024;
		printf("sending data to bank no %d, starting address %d\n", bank, startAddress);
		unsigned int address = 0;
		while(address < 1024){
             char data [4];
            data[0] = buffer[address];
            data[1] = buffer[address + 1];
            data[2] = buffer[address + 2];
            data[3] = buffer[address + 3];
			cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, SAVE_TO_EEPROM, startAddress + address, 0, data, 4, 5000);
			address += 4;
		}
        if(cnt < 0){
            fprintf(stderr, "USB error: %s\n", usb_strerror());
        }
	}else if(strcasecmp(argv[1], "rotationOffset") == 0){
        unsigned int rotationOffset = atoi(argv[2]);
        char data [8];
        cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, SET_ROTATION_OFFSET, rotationOffset, 0, data, 0, 5000);
        if(cnt < 0){
            fprintf(stderr, "USB error: %s\n", usb_strerror());
        }
	}else if(strcasecmp(argv[1], "pictureDelay") == 0){
        unsigned int pictureDelay = atoi(argv[2]);
        char data [8];
        cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, SET_PICTURE_DELAY, pictureDelay, 0, data, 0, 5000);
        if(cnt < 0){
            fprintf(stderr, "USB error: %s\n", usb_strerror());
        }
	}else if(strcasecmp(argv[1], "pictureCount") == 0){
        unsigned int pictureCount = atoi(argv[2]);
        char data [8];
        cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, SET_PICTURE_COUNT, pictureCount, 0, data, 0, 5000);
        if(cnt < 0){
            fprintf(stderr, "USB error: %s\n", usb_strerror());
        }
        
#if ENABLE_TEST
    }else if(strcasecmp(argv[1], "test") == 0){
        int i;
        srandomdev();
        for(i = 0; i < 50000; i++){
            int value = random() & 0xffff, index = random() & 0xffff;
            int rxValue, rxIndex;
            if((i+1) % 100 == 0){
                fprintf(stderr, "\r%05d", i+1);
                fflush(stderr);
            }
            cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, CUSTOM_RQ_ECHO, value, index, buffer, sizeof(buffer), 5000);
            if(cnt < 0){
                fprintf(stderr, "\nUSB error in iteration %d: %s\n", i, usb_strerror());
                break;
            }else if(cnt != 4){
                fprintf(stderr, "\nerror in iteration %d: %d bytes received instead of 4\n", i, cnt);
                break;
            }
            rxValue = ((int)buffer[0] & 0xff) | (((int)buffer[1] & 0xff) << 8);
            rxIndex = ((int)buffer[2] & 0xff) | (((int)buffer[3] & 0xff) << 8);
            if(rxValue != value || rxIndex != index){
                fprintf(stderr, "\ndata error in iteration %d:\n", i);
                fprintf(stderr, "rxValue = 0x%04x value = 0x%04x\n", rxValue, value);
                fprintf(stderr, "rxIndex = 0x%04x index = 0x%04x\n", rxIndex, index);
            }
        }
        fprintf(stderr, "\nTest completed.\n");
#endif /* ENABLE_TEST */
    }else{
        usage(argv[0]);
        exit(1);
    }
    usb_close(handle);
    return 0;
}
