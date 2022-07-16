#include "fmn_genioc_internal.h"

int fmn_genioc_cb_close(struct video_driver *driver) {
  fmn_genioc.terminate=1;
  return 0;
}

int fmn_genioc_cb_focus(struct video_driver *driver,int focus) {
  fprintf(stderr,"TODO %s %d\n",__func__,focus);
  return 0;
}

int fmn_genioc_cb_resize(struct video_driver *driver,int w,int h) {
  return 0;
}

int fmn_genioc_cb_key(struct video_driver *driver,int keycode,int value) {
  fprintf(stderr,"TODO %s 0x%08x=%d\n",__func__,keycode,value);
  return 0;
}

int fmn_genioc_cb_text(struct video_driver *driver,int codepoint) {
  fprintf(stderr,"TODO %s U+%x\n",__func__,codepoint);
  return 0;
}

int fmn_genioc_cb_mmotion(struct video_driver *driver,int x,int y) {
  fprintf(stderr,"TODO %s %d,%d\n",__func__,x,y);
  return 0;
}

int fmn_genioc_cb_mbutton(struct video_driver *driver,int btnid,int value) {
  fprintf(stderr,"TODO %s %d=%d\n",__func__,btnid,value);
  return 0;
}

int fmn_genioc_cb_mwheel(struct video_driver *driver,int dx,int dy) {
  fprintf(stderr,"TODO %s %+d,%+d\n",__func__,dx,dy);
  return 0;
}
  
int fmn_genioc_cb_pcm(struct audio_driver *driver,int16_t *v,int c) {
  //fprintf(stderr,"TODO %s %d\n",__func__,c);
  return 0;
}
  
int fmn_genioc_cb_connect(struct input_driver *driver,int devid) {
  fprintf(stderr,"TODO %s %d\n",__func__,devid);
  return 0;
}

int fmn_genioc_cb_disconnect(struct input_driver *driver,int devid) {
  fprintf(stderr,"TODO %s %d\n",__func__,devid);
  return 0;
}

int fmn_genioc_cb_event(struct input_driver *driver,int devid,int btnid,int value) {
  fprintf(stderr,"TODO %s %d.0x%08x=%d\n",__func__,devid,btnid,value);
  return 0;
}
