/**
 * Updates the keyboard color based on the information taken from lm-sensors.
 * For the MSI GE60 2PC Apache keyboard.
 * 
 * By Alfonso Arbona Gimeno - 2015 - nakerium.com
 * Based on https://github.com/gokuhs/MSI_GT_GE_Led_Enabler
 */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <hidapi.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sensors/sensors.h>

#define REFRESH_INTERVAL 15 // Time in seconds
#define DEVICE_ID 1 // Device and subdevice from where to take the temperature.
#define SUBDEV_ID 0 // Check the code (main loop) to select yours.
#define TEMP_LOW 30.0 // Temperature to be considered normal (below it's full green)
#define TEMP_HIGH 90.0 // Temperature to be considered high (higher means full red)
#define NUMBER_CPUS 4 // Number of processors in the system (see /proc/cpuinfo if you don't know)

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

hid_device *handle = NULL;

#define CLAMP(a, x, b) ((x) >= (b) ? (b) : ((x) <= (a) ? (a) : (x)))

/**
 * Sends to the handler the area / color selected. NOTE you need to commit for this applies
 */
void sendActivateArea(hid_device *handle, unsigned char area,
					  unsigned char r, unsigned char g, unsigned char b)
{
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
void commit(hid_device *handle, unsigned char mode)
{
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

void signal_callback_handler(int signum)
{
	printf("Exiting on signal %d\n", signum);

	if(handle)
	{
		hid_close(handle);
		handle = NULL;
		hid_exit();
	}

	// Terminate program
	exit(0);
}

int main(int argc, char* argv[])
{
	unsigned char r, g, b;
	struct timespec tms;
	const sensors_chip_name *chip;
	const sensors_feature *feature;
	int nr, subfeat_nr;
	double temp, used_temp, cpu_percent;
	FILE *cpufile;
	
	// Open the sensors
	if(sensors_init(NULL) != 0)
	{
		printf("Error initializing the sensors\n");
		return 1;
	}
	atexit(sensors_cleanup);
	

	// Ready to open lights
	// Open the device using the VID, PID
	handle = hid_open(0x1770, 0xff00, NULL);
	if (!handle)
	{
		printf("Unable to open MSI Led device.\n");
 		return 1;
	}
	signal(SIGINT, signal_callback_handler);
	
	while(1)
	{
		nr = 0;
		used_temp = 0;
		while((chip = sensors_get_detected_chips(NULL, &nr)) != NULL)
		{
			subfeat_nr = 0;
			while((feature = sensors_get_features(chip, &subfeat_nr)) != NULL)
			{
				// Uncomment the next printfs to get your device and subdevice ID
				//printf("%s [%d,%d] ", sensors_get_label(chip, feature), nr-1, subfeat_nr-1);
				if(sensors_get_value(chip, subfeat_nr-1, &temp) == 0)
				{
					//printf(" = %.2fºC\n", temp);
				}
				else
				{
					//printf(" = NO DATA\n");
				}
				
				if(nr-1 == DEVICE_ID && subfeat_nr-1 == SUBDEV_ID)
					used_temp = temp;
			}
		}
		
		if(used_temp <= 0)
			r = 0; // No data. Make it full green (zero red)
		else
			r =  CLAMP(0, 0xFF*(used_temp - TEMP_LOW)/(TEMP_HIGH - TEMP_LOW), 0xFF);
		
		g = 0xFF - r; // Fade from red to green
		b = 0;
		
		commit(handle, MODE_NORMAL); // You have to commit first in GE60 (?)
		sendActivateArea(handle, AREA_LEFT, r, g, b);
		
		// Get CPU info

		cpu_percent = 0.0;
		cpufile = fopen("/proc/loadavg", "r");
		if(cpufile)
		{
			fscanf(cpufile, "%lf", &cpu_percent);
			fclose(cpufile);
		}
		cpu_percent /= NUMBER_CPUS;
		r = 0;
		g = 0xFF;
		b = CLAMP(0, 0xFF * cpu_percent, 0xFF);
		printf("CPU: %.2f\n", cpu_percent * 100.0);
		
		sendActivateArea(handle, AREA_MIDDLE, r, g, b);
		sendActivateArea(handle, AREA_RIGHT, 0x00, 0x00, 0xFF);

		tms.tv_sec = REFRESH_INTERVAL;
		tms.tv_nsec = 0;
		nanosleep(&tms, NULL);
	}


	// This should never be executed tho...
	hid_close(handle);
	handle = NULL;
	hid_exit();

	return 0;
}
