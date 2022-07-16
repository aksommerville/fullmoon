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

#define FMN_GENIOC_FRAME_RATE 60 /* hz */
#define FMN_GENIOC_SLEEP_LIMIT 100000 /* us */

extern struct fmn_genioc {
  struct intf *intf;
  int terminate;
  volatile int sigc;
  uint16_t input;
  
  double starttime;
  double starttimecpu;
  int framec;
  int64_t frametime;
  int64_t nexttime;
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

double fmn_genioc_now_s();
double fmn_genioc_now_cpu_s();
int64_t fmn_genioc_now_us();
void fmn_genioc_sleep_us(int us);

#endif
