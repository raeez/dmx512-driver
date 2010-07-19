#include <stdio.h>
#include <fftw3.h>
#include <math.h>
#include <jack.h>
#include "dmx-eth.h"
#include <unistd.h>

#define WINDOW_SIZE 1024

double * in;
fftw_complex * out;
fftw_complex * in2;
double * out2;
fftw_plan ff_plan;
fftw_plan ff_plan2;

jack_client_t * jclient;
jack_port_t * j_lp;
jack_port_t * j_rp;

int ck_light;
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
/*   very_long_volume = (80*very_long_volume + long_volume) / 81; */

  //  set_light(light_data, 0, 127+(volume-long_volume)/long_volume*150, volume/very_long_volume*255, 0);
  //  output_color_triples(ck_light, light_data, 100);
  if(the_long_volume > 0.0 && the_volume/the_long_volume > 0.0) {
    scratch = (1+log(the_volume/the_long_volume)/log(2))/2.5;
    curr_volume_light = (1*curr_volume_light + scratch) / 2;
  }
  //  printf("%f ", curr_volume_light);
  set_hue_light(light_data, 25, 0, curr_volume_light, curr_volume_light);

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
  //  printf("\t%f\t", variance);
  variance_hue += 1/(1+variance)/30;
  set_hue_light(light_data, 24, variance_hue, (1+curr_volume_light)/2, 0);

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
    /* for(j = 0; j < WINDOW_SIZE; j++) { */
/*       in2[j][0] = 0; */
/*       in2[j][1] = 0; */
/*     } */
/*     for(j = WINDOW_SIZE*i/256; j < WINDOW_SIZE*(i+1)/256; j++) { */
/*       in2[j][0] = out[j][0]; */
/*       in2[j][1] = out[j][1]; */
/*       in2[WINDOW_SIZE-j-1][0] = out[WINDOW_SIZE-j-1][0]; */
/*       in2[WINDOW_SIZE-j-1][1] = out[WINDOW_SIZE-j-1][1]; */
/*     } */
/*     fftw_execute(ff_plan2); */
/*     volume[i] = 0; */
/*     for(j = 0; j < WINDOW_SIZE; j++) { */
/*       volume[i] += out2[j]*out2[j]; */
/*     } */
/*     volume[i] = sqrt(volume[i]/WINDOW_SIZE); */
    
/*     long_volume[i] = (80*long_volume[i] + volume[i])/81; */

/*     //set_hue_light(light_data, i, 127+log(volume[i]/long_volume[i])/log(2)*150, 1, 0); */
/*     band_factor = (2+log(volume[i]/long_volume[i])/log(2))/4; */
/*     global_factor = (2+log(the_volume/the_long_volume)/log(2))/8; */
/*     //set_hue_light(light_data, i, 0.2+global_factor/2, band_factor, fabs(band_factor-global_factor)); */
/*     set_light(light_data, i, 160*band_factor, 0, 0); */

/*     printf("%f\t(%f)\t", volume[i], long_volume[i]); */
    volume[i] = sqrt(pow(out[i][0], 2.0)+pow(out[i][1], 2.0))*pow(1.03, i);
    long_volume[i] = (80*long_volume[i] + volume[i])/81;
    very_long_volume[i] = (80*very_long_volume[i] + long_volume[i])/81;
    if(long_volume[i] < 0.5*max_band_volume_average) long_volume[i] = 0.5*max_band_volume_average;
    band_factor = (2 + log(volume[i]/long_volume[i])/log(2))/2;

    band_factor2 = sqrt(sqrt(pow(out[2*i][0], 2.0)+pow(out[2*i][1], 2.0))*volume[i]);
    band_factor2 = (2 + log(band_factor2/long_volume[i])/log(2))/2;

    band_factor3 = sqrt(sqrt(pow(out[3*i][0], 2.0)+pow(out[3*i][1], 2.0))*volume[i]);
    band_factor3 = (2 + log(band_factor3/long_volume[i])/log(2))/2;
    set_light(light_data, i, 160.0/255*band_factor, 160.0/255*band_factor2, 160.0/255*band_factor3);
    //printf("| %f\t%f\t%f\t", band_factor, band_factor2, band_factor3);
    /* if(volume[i] < 0.3*long_volume[i]) { */
/*       printf(" "); */
/*     } else if(volume[i] < 0.6 * long_volume[i]) { */
/*       printf("."); */
/*     } else if(volume[i] < long_volume[i]) { */
/*       printf("-"); */
/*     } else { */
/*       printf("*"); */
/*     } */
  }
  //  printf("%f", max_band_volume_average);
  //  printf("\n");
  output_color_triples(ck_light, light_data, 100);
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
  //  if(i < WINDOW_SIZE
  //  memcpy(in, lin, sizeof(jack_default_audio_sample_t)*nframes);
  //  fftw_execute(ff_plan);

  return 0;
}

void j_shutdown(void *arg) {
  
}

int main(int argc, char** argv) {
  // set up color kinetics
  ck_light = initialize_dmx("18.224.0.173");
  light_data[0] = 0;
  light_data[1] = 255;
  output_color_triples(ck_light, light_data, 4);

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

  //  scanf("Hit enter to quit\n");
  //  sleep(1);
  scanf("hi");
  while(1) {
    printf(".");
    sleep(1);
  }  
  jack_client_close(jclient);
  
  return 0;
}
