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
