#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <iostream>
#include <string>

#include "dmx-eth.h"

using namespace std;

DMX512Connection::DMX512Connection(char * ip_address) {
	if ((handle.sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
		cerr << "DMX512Connection::DMX512Connection: socket(PF_INET, SOCK_DGRAM, 0) returned -1." << endl;
		exit(1);
	}
	handle.destsa.sin_family = AF_INET;
	handle.destsa.sin_addr.s_addr = inet_addr(ip_address);
	handle.destsa.sin_port = htons(INET_UDP_PORT);
}

void DMX512Connection::dmx512(int dlen) {
	int i;
	unsigned char buffer[2048]; // 2K for good measure
	memset(buffer, 0, 2048);

	KiNET_DMXout * kdmxout = (KiNET_DMXout *)buffer;

	kdmxout->magic = INET_MAGIC;
	kdmxout->ver = INET_VERSION;
	kdmxout->type = TYPE_DMXOUT;
	kdmxout->seq = 0;

	kdmxout->port = 0;
	kdmxout->timerVal = 0;
	kdmxout->uni= -1;

	int len = sizeof(KiNET_DMXout) + dlen;

	cerr << "color kinetics datagram header size (octets): " << sizeof(KiNET_DMXout) << endl;
	cerr << "final packet size (octets): " << len << endl;

	memcpy(buffer + sizeof(KiNET_DMXout), light_data, dlen);

	if (sendto(handle.sock, buffer, len,0,(struct sockaddr *)&handle.destsa, sizeof(handle.destsa))==-1) {
		cerr << "Error when attempting to send data over dmx512!" << endl;
		exit(1);
	}
}
void DMX512Connection::output_color_light_data(int lights) {
	unsigned char buffer[2048]; // 2K for good measure
	memset(buffer, 0, 2048);

	for(int i = 0; i < lights; i++) {
		buffer[3*i+1] = light_data[3*i];
		buffer[3*i+2] = light_data[3*i+1];
		buffer[3*i+3] = light_data[3*i+2];
	}
	
	// fill the rest of the buffer
	for(int i = ((3*lights)+1); i < 512; i++) {
		buffer[i] = 0;
	}
	
	//footer
	buffer[511] = 255;
	buffer[512] = 191;

	dmx512(513); // 533 bytes total (20 byte header + 512 bytes data)
}

void DMX512Connection::set_light(int light, double r, double g, double b) {
	r = r > 0 ? (r < 1 ? r : 1) : 0;
	g = g > 0 ? (g < 1 ? g : 1) : 0;
	b = b > 0 ? (b < 1 ? b : 1) : 0;

	light_data[3 * light] = (unsigned char) (255.0 * pow(r, 0.8));
	light_data[3 * light + 1] = (unsigned char) (255.0 * pow(g, 0.8));
	light_data[3 * light + 2] = (unsigned char) (255.0 * pow(b, 0.8));
}

void DMX512Connection::set_hue_light(int light, double hue, double bright, double sat) {
	bright = bright > 0 ? (bright < 1 ? bright : 1) : 0;
	sat = sat < bright ? (sat > 0 ? sat : 0) : bright;

	set_light(light,
		(1 + cos(hue)) / 2 * (bright - sat) + sat,
		(1 + cos(hue - 2 * M_PI / 3)) / 2 * (bright - sat) + sat,
		(1 + cos(hue - 4 * M_PI / 3)) / 2 * (bright - sat) + sat);
}
