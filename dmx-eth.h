#ifndef __DMX_ETH_HEADER
#define __DMX_ETH_HEADER

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include <iostream>
#include <string>

#define INET_UDP_PORT 6038
#define INET_VERSION 0x0001
#define INET_MAGIC 0x4adc0104
#define TYPE_DMXOUT 0x0101

typedef struct {
	unsigned int magic;
	unsigned short ver;
	unsigned short type;
	unsigned int seq;
	unsigned char port;
	unsigned short timerVal;
	unsigned int uni;
	//END 20-byte header
	//START 512-byte dmx data
} KiNET_DMXout;

typedef struct {
	int sock;
	struct sockaddr_in destsa;
} DMX_Handle;

class DMX512Connection {
public:
	DMX512Connection(char * ip_addr);
	~DMX512Connection() {}

	void output_color_light_data(int lights);
	void set_light(int light_index, double red, double green, double blue);
	void set_hue_light(int light_index, double hue, double brightness, double saturation);

private:
	unsigned char light_data[512];

	DMX_Handle handle;

	void dmx512(int dlen);
};

#endif
