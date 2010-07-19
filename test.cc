#include <stdio.h>
#include <math.h>
#include "dmx-eth.h"
#include <unistd.h>
#include <iostream>

using namespace std;

unsigned char light_data[512];

void fps_sleep(int fps);

int main(int argc, char** argv) {
	
	DMX512Connection * dmx = new DMX512Connection("172.16.1.101");
	double loc = 0.0;
	char on = 1;

	while(true) {

		for(int i = 0; i < 200; i++) {
			dmx->set_hue_light(light_data, i, loc+(i==24?0:(i==25?13:i))/4.0, 1, 0.0);
		}

		fps_sleep(30);

		dmx->output_color_triples(light_data, 100);
		loc += 0.01;
	}
	return 0;
}

void fps_sleep(int fps) {
	usleep(1000000/fps);
}
