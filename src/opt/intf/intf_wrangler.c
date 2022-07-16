#include "intf.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
 
/* Delete.
 */
 
void intf_del(struct intf *intf) {
  if (!intf) return;
  if (intf->refc-->1) return;
  
  video_driver_del(intf->video);
  audio_driver_del(intf->audio);
  if (intf->inputv) {
    while (intf->inputc-->0) input_driver_del(intf->inputv[intf->inputc]);
    free(intf->inputv);
  }
  
  free(intf);
}

/* Retain.
 */
 
int intf_ref(struct intf *intf) {
  if (!intf) return -1;
  if (intf->refc<1) return -1;
  if (intf->refc==INT_MAX) return -1;
  intf->refc++;
  return 0;
}

/* Initialize video driver.
 */
 
static int intf_init_video(struct intf *intf,const char *name) {
  if (name) {
    const struct video_driver_type *type=video_driver_type_by_name(name,-1);
    if (!type) {
      fprintf(stderr,"%s: Video driver '%s' not found.\n",intf->exename,name);
      return -1;
    }
    if (!(intf->video=video_driver_new(type,&intf->delegate))) {
      fprintf(stderr,"%s: Failed to initialize video driver '%s'\n",intf->exename,name);
      return -1;
    }
  } else {
    int p=0;
    while (1) {
      const struct video_driver_type *type=video_driver_type_by_index(p);
      if (!type) {
        fprintf(stderr,"%s: Failed to initialize any video driver.\n",intf->exename);
        return -1;
      }
      if (intf->video=video_driver_new(type,&intf->delegate)) break;
      fprintf(stderr,"%s: Failed to initialize video driver '%s', looking for others.\n",intf->exename,type->name);
      p++;
    }
  }
  fprintf(stderr,"%s: Using video driver '%s'\n",intf->exename,intf->video->type->name);
  return 0;
}

/* Initialize audio driver.
 */
 
static int intf_init_audio(struct intf *intf,const char *name) {
  if (name) {
    const struct audio_driver_type *type=audio_driver_type_by_name(name,-1);
    if (!type) {
      fprintf(stderr,"%s: Audio driver '%s' not found\n",intf->exename,name);
      return -1;
    }
    if (!(intf->audio=audio_driver_new(type,&intf->delegate))) {
      fprintf(stderr,"%s: Failed to initialize audio driver '%s'\n",intf->exename,name);
      return -1;
    }
  } else {
    int p=0;
    while (1) {
      const struct audio_driver_type *type=audio_driver_type_by_index(p);
      if (!type) {
        fprintf(stderr,"%s: Failed to initialize any audio driver.\n",intf->exename);
        return -1;
      }
      if (intf->audio=audio_driver_new(type,&intf->delegate)) break;
      fprintf(stderr,"%s: Failed to initialize audio driver '%s', looking for others.\n",intf->exename,type->name);
      p++;
    }
  }
  fprintf(stderr,
    "%s: Using audio driver '%s', rate=%d, chanc=%d\n",
    intf->exename,intf->audio->type->name,intf->audio->rate,intf->audio->chanc
  );
  //TODO Should we create the synthesizer first? Who do we see about that?
  if (audio_driver_play(intf->audio,1)<0) {
    fprintf(stderr,"%s: Failed to start audio.\n",intf->exename);
    return -1;
  }
  return 0;
}

/* Initialize input drivers.
 */
 
static int intf_init_input_1(struct intf *intf,const struct input_driver_type *type) {
  if (intf->inputc>=intf->inputa) {
    int na=intf->inputa+8;
    if (na>INT_MAX/sizeof(void*)) return -1;
    void *nv=realloc(intf->inputv,sizeof(void*)*na);
    if (!nv) return -1;
    intf->inputv=nv;
    intf->inputa=na;
  }
  struct input_driver *driver=input_driver_new(type,&intf->delegate);
  if (!driver) return -1;
  intf->inputv[intf->inputc++]=driver;
  fprintf(stderr,"%s: Using input driver '%s'\n",intf->exename,type->name);
  return 0;
}
 
static int intf_init_input(struct intf *intf,const char *names) {
  if (names) {
    int namep=0;
    while (names[namep]) {
      if (names[namep]==',') { namep++; continue; }
      const char *name=names+namep;
      int namec=0;
      while (name[namec]&&(name[namec]!=',')) { namec++; namep++; }
      const struct input_driver_type *type=input_driver_type_by_name(name,namec);
      if (!type) {
        fprintf(stderr,"%s: Input driver '%.*s' not found.\n",intf->exename,namec,name);
        return -1;
      }
      if (intf_init_input_1(intf,type)<0) {
        fprintf(stderr,"%s: Failed to initialize input driver '%.*s'\n",intf->exename,namec,name);
        return -1;
      }
    }
  } else {
    int p=0;
    while (1) {
      const struct input_driver_type *type=input_driver_type_by_index(p);
      if (!type) break;
      if (intf_init_input_1(intf,type)<0) {
        fprintf(stderr,"%s: Failed to initialize input driver '%s', proceeding anyway.\n",intf->exename,type->name);
      }
      p++;
    }
  }
  return 0;
}

/* Finalize setup. (argv) overrides other delegate fields.
 */
 
static int intf_init(struct intf *intf) {
  
  intf->exename="ctm";
  if ((intf->delegate.argc>=1)&&intf->delegate.argv[0]&&intf->delegate.argv[0][0]) {
    intf->exename=intf->delegate.argv[0];
  }
  
  const char *videoname=0;
  const char *audioname=0;
  const char *inputnames=0;
  
  #define INTARG(fldname,p) { \
    int v=0; \
    int pp=p; for (;arg[pp];pp++) { \
      int digit=arg[pp]-'0'; \
      if ((digit<0)||(digit>9)||(v>INT_MAX/10)) { \
        fprintf(stderr,"%s: Failed to parse integer arg '%s'\n",intf->exename,arg); \
        return -1; \
      } \
      v*=10; \
      v+=digit; \
    } \
    intf->delegate.fldname=v; \
  }
  
  int i=1; for (;i<intf->delegate.argc;i++) {
    const char *arg=intf->delegate.argv[i];
    if (!arg||(arg[0]!='-')||(arg[1]!='-')) continue;
    
    if (!memcmp(arg,"--video=",8)) { videoname=arg+8; continue; }
    if (!memcmp(arg,"--audio=",8)) { audioname=arg+8; continue; }
    if (!memcmp(arg,"--input=",8)) { inputnames=arg+8; continue; }
    if (!memcmp(arg,"--audio-rate=",13)) { INTARG(audio_rate,13) continue; }
    if (!memcmp(arg,"--audio-chanc=",14)) { INTARG(chanc,14) continue; }
  }
  #undef INTARG
  
  if (intf_init_video(intf,videoname)<0) return -1;
  if (intf_init_input(intf,inputnames)<0) return -1;
  if (intf_init_audio(intf,audioname)<0) return -1;
  
  return 0;
}

/* New.
 */

struct intf *intf_new(const struct intf_delegate *delegate) {
  struct intf *intf=calloc(1,sizeof(struct intf));
  if (!intf) return 0;
  intf->refc=1;
  if (delegate) intf->delegate=*delegate;
  if (intf_init(intf)<0) {
    intf_del(intf);
    return 0;
  }
  return intf;
}

/* Update.
 */

int intf_update(struct intf *intf) {
  if (audio_driver_update(intf->audio)<0) return -1;
  if (video_driver_update(intf->video)<0) return -1;
  int i=intf->inputc;
  while (i-->0) {
    if (input_driver_update(intf->inputv[i])<0) return -1;
  }
  return 0;
}
