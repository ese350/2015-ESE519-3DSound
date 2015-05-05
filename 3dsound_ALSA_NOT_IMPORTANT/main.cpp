#define ALSA_PCM_NEW_HW_PARAMS_API
#include <iostream>
#include "CircularBuffer.h"
#include "WaveFileReader.h"
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <thread>
#include <stdlib.h>
static char* device = "default";
snd_pcm_t *handle;
snd_pcm_uframes_t frames;
snd_pcm_hw_params_t *params;


unsigned short buffer[512];
CircularBuffer circularBuffer(MAXSIZE+EXTRASIZE);  

void writeAnalog(int period){
  
  static int i = 0;
  while(1){
    std::cout<<circularBuffer.readOneSample()<<std::endl;
    i++;
    // frames = snd_pcm_writei(handle,buffer,MAXSIZE);
    usleep(period);
  }
}

int main() {
  int err;
  int dir = 0;
  FILE *file;
  file = fopen("sample.wav","r");
  WaveFileReader waveReader(file,false);
  err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK,0);
  if(err < 0) {
    std::cout<<"Playback open error: %s\r\n",snd_strerror(err); exit(EXIT_FAILURE);
  }
  
  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_malloc(&params);
 
  /* Fill it in with default values. */
  err = snd_pcm_hw_params_any(handle, params);
  if(err < 0){
    std::cout<<"Cannot fill with default values"<<std::endl;
    exit(1);
  }
  /* Interleaved mode */
  err = snd_pcm_hw_params_set_access(handle, params,
			       SND_PCM_ACCESS_RW_INTERLEAVED);
  if(err < 0){
    std::cout<<"Cannot set access parameters"<<std::endl;
    exit(1);
  }
  /* Signed 16-bit little-endian format */
  err = snd_pcm_hw_params_set_format(handle, params,
			       SND_PCM_FORMAT_S16_LE);
  if(err < 0) {
    std::cout<<"Cannot set parameter"<<std::endl;
    exit(1);
  }
  /* 44100 bits/second sampling rate (CD quality) */
  unsigned int val = 44100;
  err = snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, NULL);
  if(err < 0){
    std::cout<<"Cannot set rate"<<std::endl;
    exit(1);
  }
  /* Set period size to 64 frames. */
  frames = 64;
  err = snd_pcm_hw_params_set_period_size_near(handle,
					 params, &frames, NULL);
  if(err < 0){
    std::cout<<"Cannot set period size"<<std::endl;
    exit(1);
  }
  /* Write the parameters to the driver */
  /*  err = snd_pcm_hw_params(handle, params);
      if (err < 0) {
      fprintf(stderr,
      "unable to set hw parameters: %s\n",
      snd_strerror(err));
      exit(1);
      }
  */
  
  
  if((err = snd_pcm_set_params(handle,SND_PCM_FORMAT_U16_LE,SND_PCM_ACCESS_RW_INTERLEAVED,1,44100,1,500000)) < 0) {
    std::cout<<"PLayback open error: %s\n", snd_strerror(err);
    exit(EXIT_FAILURE);
  }
  bool play = true;
  size_t res;
  while(play){
    play = false;
    res = waveReader.read(buffer,MAXSIZE);
    if(res != 0)
      play = true;
    frames = snd_pcm_writei(handle,buffer,(long)sizeof(buffer));
    if(frames < 0)
      frames = snd_pcm_recover(handle, frames, 0);
    if(frames < 0){
      std::cout<<"snd_pcm_writei failed "<<snd_strerror(err)<<std::endl;
      break;
    }
    if(frames> 0 && frames < (long) sizeof(buffer))
	std::cout<<"Short write expected: "<<(long) sizeof(buffer)<<" ,wrote "<<frames<<std::endl;
  } 
 
  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);
  fclose(file); 
  while(1) {  }
}
