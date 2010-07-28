#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include <iostream>
#include <string>

#include "dmx-eth.h"

#define KINET_UDP_PORT 6038
#define KINET_VERSION 0x0001
#define KINET_MAGIC 0x4adc0104
#define KTYPE_DMXOUT 0x0101

using namespace std;

DMX512Connection::DMX512Connection(char * ip_addr) {
	DMX_Handle * handle = handles+next_handle;

	if ((handle->sock=socket(PF_INET,SOCK_DGRAM,0))==-1) {
		cerr << "could not init dmx!" << endl;
		exit(1);
	}

	handle->destsa.sin_family=AF_INET;
	handle->destsa.sin_addr.s_addr=inet_addr(ip_addr);
	handle->destsa.sin_port=htons(KINET_UDP_PORT);

	dmx_handle = next_handle++;
}

void DMX512Connection::dmx512(unsigned char * data, int dlen) {
	int i;
	int len;
	unsigned char buf[2048];
	KiNET_DMXout * kdmxout= (KiNET_DMXout *)buf;

	kdmxout->magic = KINET_MAGIC;
	kdmxout->ver = KINET_VERSION;
	kdmxout->type = KTYPE_DMXOUT;
	kdmxout->seq = 0;

	kdmxout->port = 0;
	kdmxout->timerVal = 0;
	kdmxout->uni= -1;

	len = sizeof(KiNET_DMXout)+dlen;

	cout << "color kinetics datagram header size (bytes): " << sizeof(KiNET_DMXout) << endl;
	cout << "final packet size (bytes): " << len << endl;

	memcpy(buf+sizeof(KiNET_DMXout),data,dlen);

	if (sendto(handles[dmx_handle].sock,buf,len,0,(struct sockaddr *)&handles[dmx_handle].destsa,sizeof(handles[dmx_handle].destsa))==-1) {
		cerr << "Error when attempting to send data over dmx512!" << endl;
		exit(1);
	}
}
void DMX512Connection::output_color_triples(unsigned char* triples, int lights) {
	unsigned char buf[2048]; // 2K for good measure
	buf[0] = 0;

	for(int i = 0; i < lights; i++) {
		buf[3*i+1] = triples[3*i];
		buf[3*i+2] = triples[3*i+1];
		buf[3*i+3] = triples[3*i+2];
	}
	
	// fill the rest of the buffer
	for(int i = ((3*lights)+1); i < 512; i++) {
		buf[i] = 0;
	}
	
	//footer
	buf[511] = 255;
	buf[512] = 191;

	dmx512(buf, 513); // 533 bytes total (20 byte header + 512 bytes data)
}

void DMX512Connection::set_light(unsigned char * triples, int light, double r, double g, double b) {
	r = r>0?(r<1?r:1):0;
	g = g>0?(g<1?g:1):0;
	b = b>0?(b<1?b:1):0;

	triples[3*light] = (unsigned char) (255.0*pow(r, 0.8));
	triples[3*light+1] = (unsigned char) (255.0*pow(g, 0.8));
	triples[3*light+2] = (unsigned char) (255.0*pow(b, 0.8));
}

void DMX512Connection::set_hue_light(unsigned char * triples, int light, double hue, double bright, double sat) {
	bright = bright > 0 ? (bright < 1 ? bright : 1) : 0;
	sat = sat < bright ? (sat > 0 ? sat : 0) : bright;
	set_light(triples, light,
		(1+cos(hue))/2*(bright-sat)+sat,
		(1+cos(hue-2*M_PI/3))/2*(bright-sat)+sat,
		(1+cos(hue-4*M_PI/3))/2*(bright-sat)+sat);
}
