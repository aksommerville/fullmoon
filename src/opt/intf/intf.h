/* intf.h
 * Generic I/O interfaces: video, audio, input.
 * These are used by platforms that might want to include multiple drivers, eg Linux.
 * Constrained platforms like Tiny should not use; they don't need and can't handle this kind of abstraction.
 */
 
#ifndef INTF_H
#define INTF_H

#include <stdint.h>

struct video_driver;
struct video_driver_type;

struct audio_driver;
struct audio_driver_type;

struct input_driver;
struct input_driver_type;

struct intf;
struct intf_delegate;

/* intf: Top-level driver wrangler.
 * You should instantiate one of these, and not worry about the other types.
 * We create one video, one audio, and zero or more inputs.
 * All drivers get the same delegate.
 **********************************************************************************/
 
struct intf_delegate {
  void *userdata;
  
// Request parameters that only get used at construction.
// (argv) overrides named fields, but only if you instantiate via struct intf. Individual drivers don't read argv.
  int argc;
  char **argv; // --video=NAME, --audio=NAME, --input=NAME,NAME,..., --audio-rate=INTEGER, --audio-chanc=INTEGER
  const struct video_driver_type *video_driver_type;
  const struct audio_driver_type *audio_driver_type;
  int fbw,fbh;
  int fbfmt;
  int fullscreen;
  const void *icon_rgba;
  int iconw,iconh;
  const char *title;
  int audio_rate;
  int chanc;
  
// Callbacks from video driver. Beware, some video drivers don't send events at all.
  int (*close)(struct video_driver *driver);
  int (*focus)(struct video_driver *driver,int focus);
  int (*resize)(struct video_driver *driver,int w,int h); // output size only; you shouldn't care.
  int (*key)(struct video_driver *driver,int keycode,int value);
  int (*text)(struct video_driver *driver,int codepoint);
  int (*mmotion)(struct video_driver *driver,int x,int y);
  int (*mbutton)(struct video_driver *driver,int btnid,int value);
  int (*mwheel)(struct video_driver *driver,int dx,int dy);
  
// Callback from audio driver. You must fill (v). (c) is in samples -- not frames, not bytes.
// Using the wrangler 'intf', your pcm callback will not be used. We take care of it.
  int (*pcm)(struct audio_driver *driver,int16_t *v,int c);
  
// Callbacks from input drivers. (devid) are unique across all drivers.
  int (*connect)(struct input_driver *driver,int devid);
  int (*disconnect)(struct input_driver *driver,int devid);
  int (*event)(struct input_driver *driver,int devid,int btnid,int value);
  int (*premapped_event)(struct input_driver *driver,uint16_t btnid,int value);
};
 
struct intf {
  int refc;
  const char *exename;
  struct video_driver *video;
  struct audio_driver *audio;
  struct input_driver **inputv;
  int inputc,inputa;
  struct intf_delegate delegate;
};
 
void intf_del(struct intf *intf);
int intf_ref(struct intf *intf);

struct intf *intf_new(const struct intf_delegate *delegate);

int intf_update(struct intf *intf);

/* Video.
 ********************************************************************************/
 
struct video_driver {
  const struct video_driver_type *type;
  int refc;
  struct intf_delegate delegate;
  int w,h,fullscreen;
};
 
void video_driver_del(struct video_driver *driver);
int video_driver_ref(struct video_driver *driver);

struct video_driver *video_driver_new(
  const struct video_driver_type *type,
  const struct intf_delegate *delegate
);

int video_driver_update(struct video_driver *driver);

/* You provided the format and size via delegate, and you must provide buffers of exactly that.
 */
int video_driver_swap(struct video_driver *driver,const void *fb);

int video_driver_fullscreen(struct video_driver *driver,int state);

void video_driver_suppress_screensaver(struct video_driver *driver);

/* Audio.
 ******************************************************************************/
 
struct audio_driver {
  const struct audio_driver_type *type;
  int refc;
  struct intf_delegate delegate;
  int rate,chanc;
  int playing;
};
 
void audio_driver_del(struct audio_driver *driver);
int audio_driver_ref(struct audio_driver *driver);

struct audio_driver *audio_driver_new(
  const struct audio_driver_type *type,
  const struct intf_delegate *delegate
);

int audio_driver_play(struct audio_driver *driver,int play);
int audio_driver_update(struct audio_driver *driver);
int audio_driver_lock(struct audio_driver *driver);
int audio_driver_unlock(struct audio_driver *driver);

/* Input.
 *******************************************************************************/
 
struct input_driver {
  const struct input_driver_type *type;
  int refc;
  struct intf_delegate delegate;
};

void input_driver_del(struct input_driver *driver);
int input_driver_ref(struct input_driver *driver);

struct input_driver *input_driver_new(
  const struct input_driver_type *type,
  const struct intf_delegate *delegate
);

int input_driver_update(struct input_driver *driver);

const char *input_device_get_ids(int *vid,int *pid,struct input_driver *driver,int devid);

int input_device_iterate(
  struct input_driver *driver,
  int devid,
  int (*cb)(struct input_driver *driver,int devid,int btnid,int hidusage,int value,int lo,int hi,void *userdata),
  void *userdata
);

int input_device_drop(struct input_driver *driver,int devid);

/* Types.
 ****************************************************************************/
 
struct video_driver_type {
  const char *name;
  const char *desc;
  int objlen;
  void (*del)(struct video_driver *driver);
  int (*init)(struct video_driver *driver);
  int (*update)(struct video_driver *driver);
  int (*swap)(struct video_driver *driver,const void *fb);
  int (*fullscreen)(struct video_driver *driver,int state);
  void (*suppress_screensaver)(struct video_driver *driver);
};

struct audio_driver_type {
  const char *name;
  const char *desc;
  int objlen;
  void (*del)(struct audio_driver *driver);
  int (*init)(struct audio_driver *driver);
  int (*play)(struct audio_driver *driver,int play);
  int (*update)(struct audio_driver *driver);
  int (*lock)(struct audio_driver *driver);
  int (*unlock)(struct audio_driver *driver);
};

struct input_driver_type {
  const char *name;
  const char *desc;
  int objlen;
  void (*del)(struct input_driver *driver);
  int (*init)(struct input_driver *driver);
  int (*update)(struct input_driver *driver);
  const char *(*device_get_ids)(int *vid,int *pid,struct input_driver *driver,int devid);
  int (*device_iterate)(
    struct input_driver *driver,
    int devid,
    int (*cb)(struct input_driver *driver,int devid,int btnid,int hidusage,int value,int lo,int hi,void *userdata),
    void *userdata
  );
  int (*device_drop)(struct input_driver *driver,int devid);
};

const struct video_driver_type *video_driver_type_by_index(int p);
const struct video_driver_type *video_driver_type_by_name(const char *name,int namec);
const struct audio_driver_type *audio_driver_type_by_index(int p);
const struct audio_driver_type *audio_driver_type_by_name(const char *name,int namec);
const struct input_driver_type *input_driver_type_by_index(int p);
const struct input_driver_type *input_driver_type_by_name(const char *name,int namec);

// We "allocate" naively; after 2**31 or so, we run out and give up.
int input_devid_new();

#endif
