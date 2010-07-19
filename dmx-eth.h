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

typedef struct {
	unsigned long magic;
	unsigned short ver;
	unsigned short type;
	unsigned long seq;
} KiNET_Hdr;

typedef struct {
	KiNET_Hdr hdr;
	unsigned char port;
	unsigned char flags;
	unsigned short timerVal;
	unsigned long uni;
	//DWord numChannels;
	// need to add dmx data here
} KiNET_DMXout;

typedef struct {
	int sock;
	struct sockaddr_in destsa;
} DMX_Handle;

class DMX512Connection {
public:
	DMX512Connection(char * ip_addr);
	~DMX512Connection() {}

	void output_color_triples(unsigned char* triples, int lights);
	void set_light(unsigned char * triples, int light, double r, double g, double b);
	void set_hue_light(unsigned char * triples, int light, double hue, double bright, double sat);

private:
	DMX_Handle handles[10];
	int next_handle;

	int dmx_handle;

	void dmx512(unsigned char *data, int dlen);
};

#endif