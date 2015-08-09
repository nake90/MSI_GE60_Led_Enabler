/**
 * MSI Led enabler for MSI GE60 / 70 can works with other MSIs with the same keyboard.
 * The purpose of this app is to enable the keyboard light leds when using unix based systems.
 * 
 * Christian Panadero Martinez - 2012 - Bakingcode.com - @PaNaVTEC on twitter
 * 
 * Edited by Alfonso Arbona Gimeno - 2015 - nakerium.com
 * Now works with MSI GE60 2PC Apache, maybe others too.
 */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <hidapi.h>
#include <string.h>
#include <time.h>


/** Area constants */
#define AREA_LEFT							0x01
#define AREA_MIDDLE							0x02
#define AREA_RIGHT							0x03

/** Lights modes */
#define MODE_NORMAL							0x01
#define MODE_GAMING							0x02
#define MODE_BREATHING						0x03
#define MODE_WAVE							0x04
#define MODE_DUAL_COLOR						0x05


/**
 * Sends to the handler the area / color and level selected. NOTE you need to commit for this applies
 */
void
sendActivateArea(hid_device *handle, unsigned char area, unsigned char r, unsigned char g, unsigned char b) {

	// Will send a 8 bytes array
	unsigned char data[8];
	memset(&data, 0x00, 8);
	data[0] = 0x01; // Fixed report value.
	data[1] = 0x02; // Fixed report value?

	data[2] = 0x40; // 40 = set color inputs / 41 = confirm
	data[3] = area; // 1 = left / 2 = middle / 3 = right
	data[4] = r;
	data[5] = g;
	data[6] = b;
	data[7] = 0x00; // empty

	if (hid_send_feature_report(handle, data, 9) < 0) {
		printf("Unable to send a feature report.\n");
	}

}

/**
 * Commits the lights with the modes
 */
void
commit(hid_device *handle, unsigned char mode) {

	//CONFIRMATION. This needs to be sent for confirmate all the led operations
	unsigned char data[8];
	data[0] = 0x01;
	data[1] = 0x02;

	data[2] = 0x41; // commit byte
	data[3] = mode; // current mode
	data[4] = 0x00;  
	data[5] = 0x00; 
	data[6] = 0x00;
	data[7] = 0xec;

	if (hid_send_feature_report(handle, data, 9) < 0) {
		printf("Unable to send a feature report.\n");
	}

}


int 
main(int argc, char* argv[]) {
	hid_device *handle;
	unsigned char r;
	struct timespec tms;

	// Ready to open lights
	// Open the device using the VID, PID
	handle = hid_open(0x1770, 0xff00, NULL);
	if (!handle) {
		printf("Unable to open MSI Led device.\n");
 		return 1;
	}
	
	// Sample. Edit this
	for(r = 0; r < 255 - 8; r+=8)
	{
	  commit(handle, MODE_NORMAL);
	  sendActivateArea(handle, AREA_LEFT, r, 0x00, 0x00);
	  sendActivateArea(handle, AREA_MIDDLE, r, 0xFF, 0x00);
	  sendActivateArea(handle, AREA_RIGHT, 0x00, 0x00, 0xFF);
	  
	  printf("r = %d\n", r);
	  
	  tms.tv_sec = 0;
	  tms.tv_nsec = 500* 1000 * 1000; // 500ms
	  nanosleep(&tms, NULL);
	}



	// close actual HID handler
	hid_close(handle);

	// Free static HIDAPI objects. 
	hid_exit();

	return 0;
}