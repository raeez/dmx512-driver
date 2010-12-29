#include <cstdlib>
#include <string>

#include "dmx-eth.h"

using namespace std;

void fps_sleep(int fps) {
	usleep(1000000/fps);
}

int main(void)
{
	char * address = "172.16.1.10";

	DMX512Connection * dmx = new DMX512Connection(address);

	double loc = 0.0;
	while(true) {

		for(int i = 0; i < 512; i++) {
			dmx->set_hue_light(i, loc+(i==24?0:(i==25?13:i))/4.0, 1, 0.0);
		}

		fps_sleep(30);

		dmx->output_color_light_data(512);
		loc += 0.01;
	}

	delete dmx;
	dmx = 0;
	return 0;
}
