#ifndef FMN_DRMFB_INTERNAL_H
#define FMN_DRMFB_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "opt/intf/intf.h"
#include "game/fullmoon.h"

// We're scaling in software; don't go too high.
#define FMN_DRMFB_SCALE_LIMIT 6

struct video_driver_drmfb {
  struct video_driver hdr;
  
  int fd;
  
  uint32_t connid,encid,crtcid;
  drmModeCrtcPtr crtc_restore;
  drmModeModeInfo mode;
  
  int rate; // initially requested rate, then actual
  int screenw,screenh; // screen size, also size of our framebuffers
  int dstx,dsty,dstw,dsth; // scale target within (screen)
  int scale; // 1..DRMFB_SCALE_LIMIT; fbw*scale=dstw<=screenw
  int stridewords;
  int rshift,gshift,bshift;
  
  struct fmn_drmfb_fb {
    uint32_t fbid;
    int handle;
    int size;
    void *v;
  } fbv[2];
  int fbp; // (0,1) which is attached -- draw to the other
};

#define DRIVER ((struct video_driver_drmfb*)driver)

int fmn_drmfb_init_connection(struct video_driver *driver);
int fmn_drmfb_init_buffers(struct video_driver *driver);
int fmn_drmfb_calculate_output_bounds(struct video_driver *driver);

void fmn_drmfb_scale(struct fmn_drmfb_fb *dst,struct video_driver *driver,const void *src);

#endif
