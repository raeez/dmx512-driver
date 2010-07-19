#include <stdio.h>
#include <fftw3.h>
#include <math.h>
#include <jack.h>
#include "dmx-eth.h"
#include <unistd.h>

#define WINDOW_SIZE 1024

DMX512Connection * dmx;
double * in;
fftw_complex * out;
fftw_complex * in2;
double * out2;
fftw_plan ff_plan;
fftw_plan ff_plan2;

jack_client_t * jclient;
jack_port_t * j_lp;
jack_port_t * j_rp;

unsigned char light_data[512];

void analyze(void) {
	double the_volume;
	static double the_long_volume;
	int i, j;
	double volume[24];
	static double long_volume[24];
	static double very_long_volume[24];
	double band_factor, band_factor2, band_factor3;
	double global_factor;
	static double max_band_volume_average;
	double scratch;
	static double curr_volume_light = 0.0;
	double variance;
	static double variance_hue = 0.0;

	the_volume = 0;
	for(i = 0; i < WINDOW_SIZE; i++) {
	the_volume += in[i]*in[i];
	}
	the_volume /= WINDOW_SIZE;
	the_volume = sqrt(the_volume);
	the_long_volume = (100*the_long_volume + the_volume) / 101;

	if(the_long_volume > 0.0 && the_volume/the_long_volume > 0.0) {
	scratch = (1+log(the_volume/the_long_volume)/log(2))/2.5;
	curr_volume_light = (1*curr_volume_light + scratch) / 2;
	}

	dmx->set_hue_light(light_data, 25, 0, curr_volume_light, curr_volume_light);

	fftw_execute(ff_plan);

	scratch = 0;
	for(i = 0; i < WINDOW_SIZE; i++) {
		scratch += sqrt(pow(out[i][0], 2.0)+pow(out[i][1], 2.0));
	}
	scratch /= WINDOW_SIZE;
	variance = 0.0;
	for(i = 0; i < WINDOW_SIZE; i++) {
		variance += pow(sqrt(pow(out[i][0], 2.0)+pow(out[i][1], 2.0)) - scratch, 2.0);
	}
	variance = sqrt(variance/WINDOW_SIZE);

	variance_hue += 1/(1+variance)/30;
	dmx->set_hue_light(light_data, 24, variance_hue, (1+curr_volume_light)/2, 0);

	global_factor = 0;
	for(i = 0; i < 24; i++) {
		global_factor += very_long_volume[i];
	}
	global_factor /= 24;

	scratch = 0;
	for(i = 0; i < WINDOW_SIZE; i++) {
	double scratch2 = sqrt(pow(out[i][0], 2.0) + pow(out[i][1], 2.0));
		if(scratch2 > scratch) {
		scratch = scratch2;
		}
	}
	max_band_volume_average = sqrt((80*max_band_volume_average*max_band_volume_average + scratch*scratch)/81);

	for(i = 0; i < 24; i++) {
	volume[i] = sqrt(pow(out[i][0], 2.0)+pow(out[i][1], 2.0))*pow(1.03, i);
	long_volume[i] = (80*long_volume[i] + volume[i])/81;
	very_long_volume[i] = (80*very_long_volume[i] + long_volume[i])/81;
	if(long_volume[i] < 0.5*max_band_volume_average) long_volume[i] = 0.5*max_band_volume_average;
	band_factor = (2 + log(volume[i]/long_volume[i])/log(2))/2;

	band_factor2 = sqrt(sqrt(pow(out[2*i][0], 2.0)+pow(out[2*i][1], 2.0))*volume[i]);
	band_factor2 = (2 + log(band_factor2/long_volume[i])/log(2))/2;

	band_factor3 = sqrt(sqrt(pow(out[3*i][0], 2.0)+pow(out[3*i][1], 2.0))*volume[i]);
	band_factor3 = (2 + log(band_factor3/long_volume[i])/log(2))/2;
	dmx->set_light(light_data, i, 160.0/255*band_factor, 160.0/255*band_factor2, 160.0/255*band_factor3);
	dmx->output_color_triples( light_data, 100);
	}
}

int j_receive(jack_nframes_t nframes, void * arg) {
	static int i = 0;
	int b;

	jack_default_audio_sample_t *lin = (jack_default_audio_sample_t*)jack_port_get_buffer(j_lp, nframes);
	jack_default_audio_sample_t *rin = (jack_default_audio_sample_t*)jack_port_get_buffer(j_rp, nframes);

	for(b = 0; i < WINDOW_SIZE && b < nframes; b++, i++) {
		in[i] = lin[b] + rin[b];
	}
	if(i >= WINDOW_SIZE) {
		i = 0;
		analyze();
	}

	return 0;
}

void j_shutdown(void *arg) {
  
}

int main(int argc, char** argv) {
	// set up color kinetics
	dmx = new DMX512Connection("");
	light_data[0] = 0;
	light_data[1] = 255;
	dmx->output_color_triples(light_data, 4);

	// set up fftw
	in = (double*) fftw_malloc(sizeof(double) * WINDOW_SIZE);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * WINDOW_SIZE);
	out2 = (double*) fftw_malloc(sizeof(double) * WINDOW_SIZE);
	in2 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * WINDOW_SIZE);
	ff_plan = fftw_plan_dft_r2c_1d(WINDOW_SIZE, in, out, 0);
	ff_plan2 = fftw_plan_dft_c2r_1d(WINDOW_SIZE, in2, out2, 0);

	// set up jack
	printf("Connecting to jack...\n");
		if(!(jclient = jack_client_open("timbre", JackNoStartServer, NULL))) {
			fprintf(stderr, "Cannot connect to jack.\n");
		return 1;
	}
	printf("Connected.\n");
	  
	jack_set_process_callback(jclient, j_receive, 0);
	jack_on_shutdown(jclient, j_shutdown, 0);

	printf("set jack callbacks\n");

	j_lp = jack_port_register(jclient, "in1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	j_rp = jack_port_register(jclient, "in2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	  
	if(jack_activate(jclient)) {
		fprintf(stderr, "Cannot activate jack client.\n");
		return 1;
	}
	  
	printf("activated jack client\n");

	scanf("hi");
	while(1) {
		printf(".");
		sleep(1);
	}  
	jack_client_close(jclient);
	  
	delete dmx;
	dmx = 0;
	return 0;
}
