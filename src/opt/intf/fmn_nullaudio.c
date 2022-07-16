#include "intf.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

// Arbitrary rate limits, and a hard channel count limit: only mono or stereo.
// We will clamp to these ranges -- the abstract interface lets us do that.
#define CTM_NULLAUDIO_RATE_MIN 200
#define CTM_NULLAUDIO_RATE_MAX 200000
#define CTM_NULLAUDIO_CHANC_MIN 1
#define CTM_NULLAUDIO_CHANC_MAX 2

// If we go more than 1/10 s between updates, drop some time.
// Caller should poll us at 1/60, so anything longer than this is some kind of error.
#define CTM_NULLAUDIO_UPDATE_LIMIT_US 100000

// 1/60 s of 44100 Hz in 2 channels works out to 1470 samples/frame.
// Make the buffer at least that long, and it must be a multiple of 2.
// Power of two is not required, just doing that because it feels right.
#define CTM_NULLAUDIO_BUFFER_SIZE_SAMPLES 2048

/* Current real time in microseconds.
 */
 
static int64_t get_current_time() {
  struct timeval tv={0};
  gettimeofday(&tv,0);
  return (int64_t)tv.tv_sec*1000000ll+tv.tv_usec;
}

/* Object definition.
 */
 
struct audio_driver_nullaudio {
  struct audio_driver hdr;
  int64_t pvtime;
  int16_t buffer[CTM_NULLAUDIO_BUFFER_SIZE_SAMPLES];
};

#define DRIVER ((struct audio_driver_nullaudio*)driver)

/* Cleanup.
 */
 
static void _nullaudio_del(struct audio_driver *driver) {
}

/* Init.
 */
 
static int _nullaudio_init(struct audio_driver *driver) {
       if (driver->rate<CTM_NULLAUDIO_RATE_MIN) driver->rate=CTM_NULLAUDIO_RATE_MIN;
  else if (driver->rate>CTM_NULLAUDIO_RATE_MAX) driver->rate=CTM_NULLAUDIO_RATE_MAX;
       if (driver->chanc<CTM_NULLAUDIO_CHANC_MIN) driver->chanc=CTM_NULLAUDIO_CHANC_MIN;
  else if (driver->chanc>CTM_NULLAUDIO_CHANC_MAX) driver->chanc=CTM_NULLAUDIO_CHANC_MAX;
  return 0;
}

/* Play.
 */
 
static int _nullaudio_play(struct audio_driver *driver,int play) {
  driver->playing=play;
  DRIVER->pvtime=get_current_time();
  return 0;
}

/* Update.
 */
 
static int _nullaudio_update(struct audio_driver *driver) {
  if (!driver->playing) return 0;
  
  // Check the time since last update and clamp to a panic range.
  int64_t now=get_current_time();
  int64_t elapsedus=now-DRIVER->pvtime;
  if (elapsedus<0) {
    elapsedus=0;
  } else if (elapsedus>CTM_NULLAUDIO_UPDATE_LIMIT_US) {
    elapsedus=CTM_NULLAUDIO_UPDATE_LIMIT_US;
  }
  DRIVER->pvtime=now;
  
  // Generate audio and discard it.
  // If we allow channel counts beyond 1 and 2, we would need to be careful to always request a multiple of chanc.
  if (driver->delegate.pcm) {
    int framec=(elapsedus*driver->rate)/1000000ll;
    int samplec=framec*driver->chanc;
    while (samplec>CTM_NULLAUDIO_BUFFER_SIZE_SAMPLES) {
      if (driver->delegate.pcm(driver,DRIVER->buffer,CTM_NULLAUDIO_BUFFER_SIZE_SAMPLES)<0) return -1;
      samplec-=CTM_NULLAUDIO_BUFFER_SIZE_SAMPLES;
    }
    if (samplec>0) {
      if (driver->delegate.pcm(driver,DRIVER->buffer,samplec)<0) return -1;
    }
  }
  
  return 0;
}

/* Type definition.
 */
 
const struct audio_driver_type audio_driver_type_nullaudio={
  .name="nullaudio",
  .desc="Dummy driver that pumps the synthesizer but discards its output.",
  .objlen=sizeof(struct audio_driver_nullaudio),
  .del=_nullaudio_del,
  .init=_nullaudio_init,
  .play=_nullaudio_play,
  .update=_nullaudio_update,
};
