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
	unsigned char buffer[2048]; // 2K for good measure
	memset(buffer, 0, 2048);

	DMX_Header * dmxout = (DMX_Header *)buffer;

	dmxout->magic = INET_MAGIC;
	dmxout->ver = INET_VERSION;
	dmxout->type = TYPE_DMXOUT;
	dmxout->seq = 0;

	dmxout->port = 0;
	dmxout->timerVal = 0;
	dmxout->uni = -1;

	int len = sizeof(DMX_Header) + dlen;

	printf("buffer pointer: 0x%0x\n", buffer);
	printf("color kinetics datagram header size: %d(octets)", sizeof(DMX_Header));
	printf("final packet size: %d(octets)\n", len);

	memcpy(buffer + sizeof(DMX_Header), light_data, dlen);

	if (sendto(handle.sock, buffer, len, 0, (struct sockaddr *)&handle.destsa, sizeof(handle.destsa))==-1) {
		cerr << "Error when attempting to send data!" << endl;
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
	
	//footer
	buffer[511] = 255;
	buffer[512] = 191;

	dmx512(513); // 533 octet size total (20 byte header + 512 bytes data)
}

void DMX512Connection::set_light(int light_index, double r, double g, double b) {
	r = r > 0 ? (r < 1 ? r : 1) : 0;
	g = g > 0 ? (g < 1 ? g : 1) : 0;
	b = b > 0 ? (b < 1 ? b : 1) : 0;

	light_data[3 * light_index] = (unsigned char) (255.0 * pow(r, 0.8));
	light_data[3 * light_index + 1] = (unsigned char) (255.0 * pow(g, 0.8));
	light_data[3 * light_index + 2] = (unsigned char) (255.0 * pow(b, 0.8));
}

void DMX512Connection::set_hue_light(int light, double hue, double bright, double sat) {
	bright = bright > 0 ? (bright < 1 ? bright : 1) : 0;
	sat = sat < bright ? (sat > 0 ? sat : 0) : bright;

	set_light(light,
		(1 + cos(hue)) / 2 * (bright - sat) + sat,
		(1 + cos(hue - 2 * M_PI / 3)) / 2 * (bright - sat) + sat,
		(1 + cos(hue - 4 * M_PI / 3)) / 2 * (bright - sat) + sat);
}
