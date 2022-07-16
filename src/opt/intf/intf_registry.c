#include "intf.h"
#include <limits.h>
#include <string.h>

/* The basic registries.
 * This is the one point where we need to know all the possible driver implementations.
 ****************************************************************************************/
 
// Beware, symbol is only defined if FMN_USE_*
extern const struct video_driver_type video_driver_type_nullvideo;
extern const struct video_driver_type video_driver_type_x11;
extern const struct video_driver_type video_driver_type_drmfb;
extern const struct video_driver_type video_driver_type_glx;
extern const struct video_driver_type video_driver_type_drmgx;
extern const struct video_driver_type video_driver_type_macwm;
extern const struct video_driver_type video_driver_type_mswm;
extern const struct audio_driver_type audio_driver_type_nullaudio;
extern const struct audio_driver_type audio_driver_type_alsa;
extern const struct audio_driver_type audio_driver_type_pulse;
extern const struct audio_driver_type audio_driver_type_macaudio;
extern const struct audio_driver_type audio_driver_type_msaudio;
extern const struct input_driver_type input_driver_type_evdev;
extern const struct input_driver_type input_driver_type_machid;
extern const struct input_driver_type input_driver_type_mshid;
 
static const struct video_driver_type *video_driver_typev[]={
#if FMN_USE_x11
  &video_driver_type_x11,
#endif
#if FMN_USE_drmfb
  &video_driver_type_drmfb,
#endif
#if FMN_USE_glx
  &video_driver_type_glx,
#endif
#if FMN_USE_drmgx
  &video_driver_type_drmgx,
#endif
#if FMN_USE_macwm
  &video_driver_type_macwm,
#endif
#if FMN_USE_mswm
  &video_driver_type_mswm,
#endif
  &video_driver_type_nullvideo,
};

static const struct audio_driver_type *audio_driver_typev[]={
#if FMN_USE_alsa
  &audio_driver_type_alsa,
#endif
#if FMN_USE_pulse
  &audio_driver_type_pulse,
#endif
#if FMN_USE_macaudio
  &audio_driver_type_macaudio,
#endif
#if FMN_USE_msaudio
  &audio_driver_type_msaudio,
#endif
  &audio_driver_type_nullaudio,
};

static const struct input_driver_type *input_driver_typev[]={
#if FMN_USE_evdev
  &input_driver_type_evdev,
#endif
#if FMN_USE_machid
  &input_driver_type_machid,
#endif
#if FMN_USE_mshid
  &input_driver_type_mshid,
#endif
};
 
/* Public access to registries.
 **********************************************************************************/
 
#define DRIVER_TYPE(t) \
  const struct t##_driver_type *t##_driver_type_by_index(int p) { \
    if (p<0) return 0; \
    int c=sizeof(t##_driver_typev)/sizeof(void*); \
    if (p>=c) return 0; \
    return t##_driver_typev[p]; \
  } \
  const struct t##_driver_type *t##_driver_type_by_name(const char *name,int namec) { \
    if (!name) return 0; \
    if (namec<0) { namec=0; while (name[namec]) namec++; } \
    const struct t##_driver_type **p=t##_driver_typev; \
    int i=sizeof(t##_driver_typev)/sizeof(void*); \
    for (;i-->0;p++) { \
      if (memcmp((*p)->name,name,namec)) continue; \
      if ((*p)->name[namec]) continue; \
      return *p; \
    } \
    return 0; \
  }
  
DRIVER_TYPE(video)
DRIVER_TYPE(audio)
DRIVER_TYPE(input)

#undef DRIVER_TYPE

/* Input devid registry.
 *************************************************************************/
 
static int input_devid_next=1;

int input_devid_new() {
  if (input_devid_next==INT_MAX) return -1;
  return input_devid_next++;
}
