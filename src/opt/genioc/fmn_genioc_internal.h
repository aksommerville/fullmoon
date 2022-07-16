#ifndef FMN_GENIOC_INTERNAL_H
#define FMN_GENIOC_INTERNAL_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "game/fullmoon.h"

#if FMN_USE_intf
  #include "opt/intf/intf.h"
#else
  #error "'intf' unit required"
#endif

extern struct fmn_genioc {
  struct intf *intf;
  int terminate;
  volatile int sigc;
  uint16_t input;
} fmn_genioc;

int fmn_genioc_cb_close(struct video_driver *driver);
int fmn_genioc_cb_focus(struct video_driver *driver,int focus);
int fmn_genioc_cb_resize(struct video_driver *driver,int w,int h);
int fmn_genioc_cb_key(struct video_driver *driver,int keycode,int value);
int fmn_genioc_cb_text(struct video_driver *driver,int codepoint);
int fmn_genioc_cb_mmotion(struct video_driver *driver,int x,int y);
int fmn_genioc_cb_mbutton(struct video_driver *driver,int btnid,int value);
int fmn_genioc_cb_mwheel(struct video_driver *driver,int dx,int dy);
  
int fmn_genioc_cb_pcm(struct audio_driver *driver,int16_t *v,int c);
  
int fmn_genioc_cb_connect(struct input_driver *driver,int devid);
int fmn_genioc_cb_disconnect(struct input_driver *driver,int devid);
int fmn_genioc_cb_event(struct input_driver *driver,int devid,int btnid,int value);

#endif
