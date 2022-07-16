#include "fmn_genioc_internal.h"

/* General window manager events.
 * Mostly not interesting.
 */

int fmn_genioc_cb_close(struct video_driver *driver) {
  fmn_genioc.terminate=1;
  return 0;
}

int fmn_genioc_cb_focus(struct video_driver *driver,int focus) {
  return 0;
}

int fmn_genioc_cb_resize(struct video_driver *driver,int w,int h) {
  return 0;
}

/* Keyboard input.
 */
 
static struct fmn_genioc_keymap {
  int keycode;
  uint16_t btnid;
} fmn_genioc_keymapv[]={
// Must sort manually by (keycode).
  {0x00070004,FMN_BUTTON_LEFT},//a
  {0x00070007,FMN_BUTTON_RIGHT},//d
  {0x00070016,FMN_BUTTON_DOWN},//s
  {0x0007001a,FMN_BUTTON_UP},//w
  {0x0007001b,FMN_BUTTON_B},//x
  {0x0007001d,FMN_BUTTON_A},//z
  {0x00070036,FMN_BUTTON_B},//comma
  {0x00070037,FMN_BUTTON_A},//dot
  {0x0007004f,FMN_BUTTON_RIGHT},//right arrow
  {0x00070050,FMN_BUTTON_LEFT},//left arrow
  {0x00070051,FMN_BUTTON_DOWN},//down arrow
  {0x00070052,FMN_BUTTON_UP},//up arrow
  {0x00070058,FMN_BUTTON_B},//kpenter
  {0x0007005a,FMN_BUTTON_DOWN},//kp2
  {0x0007005c,FMN_BUTTON_LEFT},//kp4
  {0x0007005d,FMN_BUTTON_DOWN},//kp5
  {0x0007005e,FMN_BUTTON_RIGHT},//kp6
  {0x00070060,FMN_BUTTON_UP},//kp8
  {0x00070062,FMN_BUTTON_A},//kp0
};

int fmn_genioc_cb_key(struct video_driver *driver,int keycode,int value) {

  // Look for special stateless keys.
  if (value==1) switch (keycode) {
    case 0x00070029: fmn_genioc.terminate=1; return 0; // ESC
    case 0x00070044: video_driver_fullscreen(fmn_genioc.intf->video,!fmn_genioc.intf->video->fullscreen); return 0; // F11
  }
  
  if (value==2) return 0; // key-repeat is not interesting.
  
  int lo=0,hi=sizeof(fmn_genioc_keymapv)/sizeof(struct fmn_genioc_keymap),p=-1;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
         if (keycode<fmn_genioc_keymapv[ck].keycode) hi=ck;
    else if (keycode>fmn_genioc_keymapv[ck].keycode) lo=ck+1;
    else { p=ck; break; }
  }
  if (p<0) return 0; // not a mapped key
  uint16_t btnid=fmn_genioc_keymapv[p].btnid;
  
  if (value) {
    if (fmn_genioc.input&btnid) return 0;
    fmn_genioc.input|=btnid;
  } else {
    if (!(fmn_genioc.input&btnid)) return 0;
    fmn_genioc.input&=~btnid;
  }
  
  return 0;
}

/* Text input. Probably not going to use.
 */

int fmn_genioc_cb_text(struct video_driver *driver,int codepoint) {
  return 0;
}

/* Mouse input. Definitely not going to use. Why did I write all this?
 */

int fmn_genioc_cb_mmotion(struct video_driver *driver,int x,int y) {
  return 0;
}

int fmn_genioc_cb_mbutton(struct video_driver *driver,int btnid,int value) {
  return 0;
}

int fmn_genioc_cb_mwheel(struct video_driver *driver,int dx,int dy) {
  return 0;
}

/* Generate PCM. Probably ought to remove this, but punt that. Maybe there will be audio eventually.
 */
  
int fmn_genioc_cb_pcm(struct audio_driver *driver,int16_t *v,int c) {
  memset(v,0,c<<1);
  return 0;
}

/* Joystick events. TODO
 */
  
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
