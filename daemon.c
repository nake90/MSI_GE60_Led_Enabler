/**
 * Updates the keyboard color based on the information taken from lm-sensors.
 * For the MSI GE60 2PC Apache keyboard.
 * 
 * By Alfonso Arbona Gimeno - 2015 - nakerium.com
 * Based on https://github.com/gokuhs/MSI_GT_GE_Led_Enabler
 */

#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <hidapi.h>
#include <time.h>
#include <sensors/sensors.h>

#define REFRESH_INTERVAL 10 // Time in seconds
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
int sendActivateArea(hid_device *handle, unsigned char area,
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

	if (hid_send_feature_report(handle, data, 9) < 0)
	{
		syslog(LOG_WARNING, "Unable to send a feature report.\n");
		return -1;
	}
	
	return 0;
}

/**
 * Commits the lights with the modes
 */
int commit(hid_device *handle, unsigned char mode)
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

	if (hid_send_feature_report(handle, data, 9) < 0)
	{
		syslog(LOG_WARNING, "Unable to send a feature report.\n");
		return -1;
	}

	return 0;
}

/**
 * Returns the free ram as a percentage [0.0 to 1.0]
 */
float get_ram_free(void)
{
	char buffer[1024];
	char *ptr;
	unsigned long mem = 0, free = 0;
	FILE *f = fopen("/proc/meminfo", "r");
	if(!f)return 0.0f;

	while(fgets(buffer, 1024, f) != NULL)
	{
		if(strncmp(buffer, "MemTotal:", 9) == 0)
		{
			ptr = buffer + 10;
			mem = atol(ptr);
		}

		if(strncmp(buffer, "MemFree:", 8) == 0)
		{
			ptr = buffer + 9;
			free = atol(ptr);
		}
	}

	fclose(f);

	//printf("mem: %ld, free: %ld\n", mem, free);

	if(mem <= 0 || free < 0) return 0.0;
	return (float)free / (float)mem;
}

/**
 * Main loop code
 */
void process(void)
{
	unsigned char r, g, b;
	const sensors_chip_name *chip;
	const sensors_feature *feature;
	int nr, subfeat_nr;
	double temp, used_temp, cpu_percent, ram_free_percent;
	FILE *cpufile;
	
	if(!handle)
	{
		// Open the device using the VID, PID
		handle = hid_open(0x1770, 0xff00, NULL);
		if (!handle)
		{
			syslog(LOG_WARNING, "Unable to open MSI Led device.\n");
			return;
		}
	}
	
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
	
	if(commit(handle, MODE_NORMAL) != 0) // You have to commit first in GE60 (?)
	{
		handle = NULL;
		return;
	}
	if(sendActivateArea(handle, AREA_LEFT, r, g, b) != 0)
	{
		handle = NULL;
		return;
	}
	
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
	//printf("CPU: %.2f\n", cpu_percent * 100.0);
	
	if(sendActivateArea(handle, AREA_MIDDLE, r, g, b) != 0)
	{
		handle = NULL;
		return;
	}
	
	// Get RAM info
	ram_free_percent = get_ram_free();

	//printf("RAM: %.2lf\n", ram_used_percent);

	b = CLAMP(0, 0xFF * ram_free_percent, 0xFF);
	r = 0xFF - b;
	g = 0;
	
	if(sendActivateArea(handle, AREA_RIGHT, r, g, b) != 0)
	{
		handle = NULL;
		return;
	}
}





void close_daemon_callback(void)
{
	syslog(LOG_NOTICE, "Closing daemon\n");
	closelog();
}


void signal_callback_handler(int signum)
{
	syslog(LOG_NOTICE, "Catched signal %d", signum);
	switch(signum)
	{
		case SIGHUP:
			// Refresh
			break;
		case SIGTERM:
			hid_close(handle);
			handle = NULL;
			hid_exit();
			exit(EXIT_SUCCESS);
			break;
	}
}

int main(int argc, char *argv[])
{
	pid_t pid, sid;

	// Fork
	pid = fork();
	if(pid < 0)
	{
		exit(EXIT_FAILURE);
	}

	// Close the parent process
	if(pid > 0)
	{
		exit(EXIT_SUCCESS);
	}

	// Change the file mask
	umask(027); // mode 750

	// Open log
	setlogmask(LOG_UPTO(LOG_NOTICE));
	openlog("msige60d", LOG_CONS | LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_DAEMON);

	syslog(LOG_INFO, "Starting msige60d");

	// Create a new session id
	sid = setsid();
	if(sid < 0)
	{
		syslog(LOG_ERR, "setsid failed");
		exit(EXIT_FAILURE);
	}

	// Change directory
	if(chdir("/tmp")  < 0)
	{
		syslog(LOG_ERR, "chdir failed");
		exit(EXIT_FAILURE);
	}

	// Close standard files
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	// Check for the lock file
	char buffer[16];
	int lfp = open("/run/lock/msige60d.lock", O_RDWR|O_CREAT, 0640);
	if(lfp < 0)
	{
		syslog(LOG_ERR, "Can't open /run/lock/msige60d.lock");
		exit(EXIT_FAILURE);
	}

	if(lockf(lfp, F_TLOCK, 0) < 0)
	{
		syslog(LOG_ERR, "Can't lock /run/lock/msige60d.lock. Already running?");
		exit(EXIT_FAILURE);
	}

	// Record the PID
	snprintf(buffer, 15, "%d\n", getpid());
	write(lfp, buffer, strlen(buffer));

	signal(SIGCHLD, SIG_IGN); // Ignore child
	signal(SIGTSTP, SIG_IGN); // Ignore tty signals
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	// Catch signals
	signal(SIGHUP, signal_callback_handler);
	signal(SIGTERM, signal_callback_handler);


	// Initialize daemon
	atexit(close_daemon_callback);
	
	// Open the sensors
	if(sensors_init(NULL) != 0)
	{
		syslog(LOG_ERR, "Error initializing the sensors\n");
		exit(EXIT_FAILURE);
	}
	atexit(sensors_cleanup);
	

	// Main loop
	while(1)
	{
		process();
		sleep(REFRESH_INTERVAL);
	}

	closelog();
	return(EXIT_SUCCESS);
}

