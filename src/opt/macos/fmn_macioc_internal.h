#ifndef FMN_MACIOC_INTERNAL_H
#define FMN_MACIOC_INTERNAL_H

#include "game/fullmoon.h"
#include "opt/intf/intf.h"
#include <string.h>

#ifdef __OBJC__
#include <Cocoa/Cocoa.h>
@interface AKAppDelegate : NSObject <NSApplicationDelegate> {
}
@end
#endif

#define FMN_MACIOC_FRAME_RATE 60
#define FMN_MACIOC_SLEEP_LIMIT 100000 /* us */

extern struct fmn_macioc {
  int init;
  int terminate;
  int update_in_progress;
  struct intf *intf;
  uint16_t input;
  int argc; // stashed here so we can examine in a more nested context (fmn_macioc_init)
  char **argv;
  
  double starttime;
  double starttimecpu;
  int framec;
  int64_t frametime;
  int64_t nexttime;
} fmn_macioc;

void fmn_macioc_abort(const char *fmt,...);
int fmn_macioc_call_init();
void fmn_macioc_call_quit();

int fmn_macioc_cb_close(struct video_driver *driver);
int fmn_macioc_cb_focus(struct video_driver *driver,int focus);
int fmn_macioc_cb_resize(struct video_driver *driver,int w,int h);
int fmn_macioc_cb_key(struct video_driver *driver,int keycode,int value);
int fmn_macioc_cb_text(struct video_driver *driver,int codepoint);
int fmn_macioc_cb_mmotion(struct video_driver *driver,int x,int y);
int fmn_macioc_cb_mbutton(struct video_driver *driver,int btnid,int value);
int fmn_macioc_cb_mwheel(struct video_driver *driver,int dx,int dy);
  
int fmn_macioc_cb_pcm(struct audio_driver *driver,int16_t *v,int c);
  
int fmn_macioc_cb_connect(struct input_driver *driver,int devid);
int fmn_macioc_cb_disconnect(struct input_driver *driver,int devid);
int fmn_macioc_cb_event(struct input_driver *driver,int devid,int btnid,int value);
int fmn_macioc_cb_premapped_event(struct input_driver *driver,uint16_t btnid,int value);

double fmn_macioc_now_s();
double fmn_macioc_now_cpu_s();
int64_t fmn_macioc_now_us();
void fmn_macioc_sleep_us(int us);

#endif
