#include "fmn_alsa_internal.h"

// Arbitrary sanity limits on rate and chanc.
// (well, chanc_min==1 is a pretty firm requirement).
#define FMN_ALSA_RATE_MIN 200
#define FMN_ALSA_RATE_MAX 200000
#define FMN_ALSA_CHANC_MIN 1
#define FMN_ALSA_CHANC_MAX 8

/* Cleanup.
 */
 
static void _alsa_del(struct audio_driver *driver) {
  DRIVER->ioabort=1;
  if (DRIVER->iothd&&!DRIVER->cberror) {
    pthread_cancel(DRIVER->iothd);
    pthread_join(DRIVER->iothd,0);
  }
  pthread_mutex_destroy(&DRIVER->iomtx);
  if (DRIVER->hwparams) snd_pcm_hw_params_free(DRIVER->hwparams);
  if (DRIVER->alsa) snd_pcm_close(DRIVER->alsa);
  if (DRIVER->buf) free(DRIVER->buf);
}

/* I/O thread.
 */

static void *_alsa_iothd(void *userdata) {
  struct audio_driver *driver=userdata;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);
  while (1) {
    pthread_testcancel();

    if (driver->playing) {
      if (pthread_mutex_lock(&DRIVER->iomtx)) {
        DRIVER->cberror=1;
        return 0;
      }
      driver->delegate.pcm(driver,DRIVER->buf,DRIVER->bufc_samples);
      pthread_mutex_unlock(&DRIVER->iomtx);
      if (DRIVER->ioabort) return 0;
    } else {
      memset(DRIVER->buf,0,DRIVER->bufc_samples<<1);
    }

    int16_t *samplev=DRIVER->buf;
    int samplep=0,samplec=DRIVER->bufc;
    while (samplep<samplec) {
      pthread_testcancel();
      int err=snd_pcm_writei(DRIVER->alsa,samplev+samplep,samplec-samplep);
      if (DRIVER->ioabort) return 0;
      if (err<=0) {
        if ((err=snd_pcm_recover(DRIVER->alsa,err,0))<0) {
          DRIVER->cberror=1;
          return 0;
        }
        break;
      }
      samplep+=err*driver->chanc;
    }
  }
  return 0;
}

/* Init.
 */
 
static int _alsa_init(struct audio_driver *driver) {
  
       if (driver->rate<FMN_ALSA_RATE_MIN) driver->rate=FMN_ALSA_RATE_MIN;
  else if (driver->rate>FMN_ALSA_RATE_MAX) driver->rate=FMN_ALSA_RATE_MAX;
       if (driver->chanc<FMN_ALSA_CHANC_MIN) driver->chanc=FMN_ALSA_CHANC_MIN;
  else if (driver->chanc>FMN_ALSA_CHANC_MAX) driver->chanc=FMN_ALSA_CHANC_MAX;
  const char *device="default";//TODO
  
  int buffer_size_limit=driver->rate/30;
  int buffer_size=256;
  while (1) {
    int next_size=buffer_size<<1;
    if (next_size>=buffer_size_limit) break;
    buffer_size=next_size;
  }

  if (
    (snd_pcm_open(&DRIVER->alsa,device,SND_PCM_STREAM_PLAYBACK,0)<0)||
    (snd_pcm_hw_params_malloc(&DRIVER->hwparams)<0)||
    (snd_pcm_hw_params_any(DRIVER->alsa,DRIVER->hwparams)<0)||
    (snd_pcm_hw_params_set_access(DRIVER->alsa,DRIVER->hwparams,SND_PCM_ACCESS_RW_INTERLEAVED)<0)||
    (snd_pcm_hw_params_set_format(DRIVER->alsa,DRIVER->hwparams,SND_PCM_FORMAT_S16)<0)||
    (snd_pcm_hw_params_set_rate_near(DRIVER->alsa,DRIVER->hwparams,&driver->rate,0)<0)||
    (snd_pcm_hw_params_set_channels_near(DRIVER->alsa,DRIVER->hwparams,&driver->chanc)<0)||
    (snd_pcm_hw_params_set_buffer_size(DRIVER->alsa,DRIVER->hwparams,buffer_size)<0)||
    (snd_pcm_hw_params(DRIVER->alsa,DRIVER->hwparams)<0)
  ) return -1;
  
  if (snd_pcm_nonblock(DRIVER->alsa,0)<0) return -1;
  if (snd_pcm_prepare(DRIVER->alsa)<0) return -1;

  DRIVER->bufc=buffer_size;
  DRIVER->bufc_samples=DRIVER->bufc*driver->chanc;
  if (!(DRIVER->buf=malloc(DRIVER->bufc_samples*2))) return -1;

  pthread_mutexattr_t mattr;
  pthread_mutexattr_init(&mattr);
  pthread_mutexattr_settype(&mattr,PTHREAD_MUTEX_RECURSIVE);
  if (pthread_mutex_init(&DRIVER->iomtx,&mattr)) return -1;
  pthread_mutexattr_destroy(&mattr);
  if (pthread_create(&DRIVER->iothd,0,_alsa_iothd,driver)) return -1;
  
  return 0;
}

/* Play, lock.
 */
 
static int _alsa_play(struct audio_driver *driver,int play) {
  if (play&&!driver->delegate.pcm) return -1;
  driver->playing=play;
  return 0;
}

static int _alsa_lock(struct audio_driver *driver) {
  if (pthread_mutex_lock(&DRIVER->iomtx)) return -1;
  return 0;
}

static int _alsa_unlock(struct audio_driver *driver) {
  if (pthread_mutex_unlock(&DRIVER->iomtx)) return -1;
  return 0;
}

/* Type definition.
 */
 
const struct audio_driver_type audio_driver_type_alsa={
  .name="alsa",
  .desc="Audio for Linux. Prefer 'pulse' for desktop environments.",
  .objlen=sizeof(struct audio_driver_alsa),
  .del=_alsa_del,
  .init=_alsa_init,
  .play=_alsa_play,
  .lock=_alsa_lock,
  .unlock=_alsa_unlock,
};
