#ifndef FMN_ALSA_INTERNAL_H
#define FMN_ALSA_INTERNAL_H

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <sys/poll.h>
#include <alsa/asoundlib.h>

#if FMN_USE_intf
  #include "opt/intf/intf.h"
#else
  #error "'intf' unit required"
#endif

struct audio_driver_alsa {
  struct audio_driver hdr;

  snd_pcm_t *alsa;
  snd_pcm_hw_params_t *hwparams;

  int hwbuffersize;
  int bufc; // frames
  int bufc_samples;
  int16_t *buf;

  pthread_t iothd;
  pthread_mutex_t iomtx;
  int ioabort;
  int cberror;
};

#define DRIVER ((struct audio_driver_alsa*)driver)

#endif
