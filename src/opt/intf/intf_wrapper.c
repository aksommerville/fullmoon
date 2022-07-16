#include "intf.h"
#include <stdlib.h>
#include <limits.h>

#define NEW_DEL_REF(t,preinit) \
  void t##_driver_del(struct t##_driver *driver) { \
    if (!driver) return; \
    if (driver->refc-->1) return; \
    if (driver->type->del) driver->type->del(driver); \
    free(driver); \
  } \
  int t##_driver_ref(struct t##_driver *driver) { \
    if (!driver) return -1; \
    if (driver->refc<1) return -1; \
    if (driver->refc==INT_MAX) return -1; \
    driver->refc++; \
    return 0; \
  } \
  struct t##_driver *t##_driver_new( \
    const struct t##_driver_type *type, \
    const struct intf_delegate *delegate \
  ) { \
    if (!type||(type->objlen<(int)sizeof(struct t##_driver))) return 0; \
    struct t##_driver *driver=calloc(1,type->objlen); \
    if (!driver) return 0; \
    driver->refc=1; \
    driver->type=type; \
    if (delegate) driver->delegate=*delegate; \
    preinit \
    if (type->init) { \
      if (type->init(driver)<0) { \
        t##_driver_del(driver); \
        return 0; \
      } \
    } \
    return driver; \
  }
  
/* Video.
 */
 
NEW_DEL_REF(video,{
  driver->w=driver->delegate.fbw;
  driver->h=driver->delegate.fbh;
  driver->fullscreen=driver->delegate.fullscreen;
})

int video_driver_update(struct video_driver *driver) {
  if (!driver->type->update) return 0;
  return driver->type->update(driver);
}

int video_driver_swap(struct video_driver *driver,const void *fb) {
  if (!driver->type->swap) return 0;
  return driver->type->swap(driver,fb);
}

int video_driver_fullscreen(struct video_driver *driver,int state) {
  if (driver->type->fullscreen) {
    if (driver->type->fullscreen(driver,state)<0) return -1;
  }
  return driver->fullscreen;
}

void video_driver_suppress_screensaver(struct video_driver *driver) {
  if (driver->type->suppress_screensaver) driver->type->suppress_screensaver(driver);
}

/* Audio.
 */

NEW_DEL_REF(audio,{
  driver->rate=driver->delegate.audio_rate;
  driver->chanc=driver->delegate.chanc;
})

int audio_driver_play(struct audio_driver *driver,int play) {
  if (play&&driver->playing) return 0;
  if (!play&&!driver->playing) return 0;
  if (!driver->type->play) return -1;
  return driver->type->play(driver,play);
}

int audio_driver_update(struct audio_driver *driver) {
  if (!driver->type->update) return 0;
  return driver->type->update(driver);
}

int audio_driver_lock(struct audio_driver *driver) {
  if (!driver->type->lock) return 0;
  return driver->type->lock(driver);
}

int audio_driver_unlock(struct audio_driver *driver) {
  if (!driver->type->unlock) return 0;
  return driver->type->unlock(driver);
}

/* Input.
 */
 
NEW_DEL_REF(input,{})

int input_driver_update(struct input_driver *driver) {
  if (!driver->type->update) return 0;
  return driver->type->update(driver);
}

const char *input_device_get_ids(int *vid,int *pid,struct input_driver *driver,int devid) {
  if (!driver->type->device_get_ids) return 0;
  return driver->type->device_get_ids(vid,pid,driver,devid);
}

int input_device_iterate(
  struct input_driver *driver,
  int devid,
  int (*cb)(struct input_driver *driver,int devid,int btnid,int hidusage,int value,int lo,int hi,void *userdata),
  void *userdata
) {
  if (!driver->type->device_iterate) return -1;
  return driver->type->device_iterate(driver,devid,cb,userdata);
}

int input_device_drop(struct input_driver *driver,int devid) {
  if (!driver->type->device_drop) return -1;
  return driver->type->device_drop(driver,devid);
}
