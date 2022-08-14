/* fmn_macwm_driver.c
 * Ideally this would be a thin pass-thru connecting macwm and Fullmoon's intf.
 * We must deal with the final scale-up tho; that was a completely separate concern where i copied macwm from.
 */

#include "fmn_macwm_internal.h"
#include <stdlib.h>

/* Instance definition.
 */

struct video_driver_macwm {
  struct video_driver hdr;
  uint8_t *fbcvt; // RGBA if not null (if null, assume provided framebuffer is RGBA)
};

#define DRIVER ((struct video_driver_macwm*)driver)

/* Cleanup.
 */

static void _macwm_del(struct video_driver *driver) {
  if (DRIVER->fbcvt) free(DRIVER->fbcvt);
  fmn_macwm_quit();
}

/* Examine requested framebuffer format, prep our conversion buffer if needed.
 */

static int macwm_init_framebuffer(struct video_driver *driver) {
  if (driver->delegate.fbw<1) return -1;
  if (driver->delegate.fbh<1) return -1;
  switch (driver->delegate.fbfmt) {
    case FMN_IMGFMT_thumby: {
        if (!(DRIVER->fbcvt=malloc(driver->delegate.fbw*driver->delegate.fbh*4))) return -1;
      } return 0;
    case FMN_IMGFMT_bgr565be: {
        if (!(DRIVER->fbcvt=malloc(driver->delegate.fbw*driver->delegate.fbh*4))) return -1;
      } return 0;
    case FMN_IMGFMT_bgr332: {
        if (!(DRIVER->fbcvt=malloc(driver->delegate.fbw*driver->delegate.fbh*4))) return -1;
      } return 0;
    case FMN_IMGFMT_rgba8888: return 0;
    // Not supported: ya11, y1, argb4444be, and anything else we add after that.
  }
  return -1;
}

/* Init.
 */

static int _macwm_init(struct video_driver *driver) {

  if (fmn_macwm_init(
    driver->delegate.fbw,driver->delegate.fbh,
    driver->delegate.fullscreen,
    driver->delegate.title,
    &driver->delegate,driver
  )<0) return -1;

  if (macwm_init_framebuffer(driver)<0) {
    fprintf(stderr,
      "%s: Failed to initialize framebuffer for format %d, size %d,%d.\n",
      __func__,driver->delegate.fbfmt,driver->delegate.fbw,driver->delegate.fbh
    );
    return -1;
  }

  return 0;
}

/* rgba from thumby's weird format.
 */

static const void *fmn_macwm_cvt_rgba_thumby(struct video_driver *driver,const uint8_t *srcrow) {
  uint8_t srcmask=0x01;
  uint8_t *dstp=DRIVER->fbcvt;
  int yi=driver->delegate.fbh;
  for (;yi-->0;) {
    const uint8_t *srcp=srcrow;
    int xi=driver->delegate.fbw;
    for (;xi-->0;srcp++,dstp+=4) {
      if ((*srcp)&srcmask) {
        dstp[0]=dstp[1]=dstp[2]=0xff;
      } else {
        dstp[0]=dstp[1]=dstp[2]=0x00;
      }
      dstp[3]=0xff;
    }
    if (srcmask==0x80) {
      srcmask=0x01;
      srcrow+=driver->delegate.fbw;
    } else {
      srcmask<<=1;
    }
  }
  return DRIVER->fbcvt;
}

/* rgba from bgr565be
 */

static const void *fmn_macwm_cvt_rgba_bgr565be(struct video_driver *driver,const uint8_t *srcrow) {
  int srcstride=driver->delegate.fbw<<1;
  int dststride=driver->delegate.fbw<<2;
  uint8_t *dstrow=DRIVER->fbcvt;
  int yi=driver->delegate.fbh;
  for (;yi-->0;srcrow+=srcstride,dstrow+=dststride) {
    const uint8_t *srcp=srcrow;
    uint8_t *dstp=dstrow;
    int xi=driver->delegate.fbw;
    for (;xi-->0;srcp+=2,dstp+=4) {
      uint8_t r=srcp[1]<<3; r|=r>>5;
      uint8_t g=(srcp[0]<<5)|((srcp[1]>>3)&0x1c); g|=g>>6;
      uint8_t b=srcp[0]&0xf8; b|=b>>5;
      dstp[0]=r;
      dstp[1]=g;
      dstp[2]=b;
      dstp[3]=0xff;
    }
  }
  return DRIVER->fbcvt;
}

/* rgba from bgr332
 */

static const void *fmn_macwm_cvt_rgba_bgr332(struct video_driver *driver,const uint8_t *srcrow) {
  int srcstride=driver->delegate.fbw;
  int dststride=driver->delegate.fbw<<2;
  uint8_t *dstrow=DRIVER->fbcvt;
  int yi=driver->delegate.fbh;
  for (;yi-->0;srcrow+=srcstride,dstrow+=dststride) {
    const uint8_t *srcp=srcrow;
    uint8_t *dstp=dstrow;
    int xi=driver->delegate.fbw;
    for (;xi-->0;srcp+=1,dstp+=4) {
      uint8_t b=(*srcp)&0xe0; b|=b>>3; b|=b>>6;
      uint8_t g=(*srcp)&0x1c; g|=g<<3; g|=g>>6;
      uint8_t r=(*srcp)&0x03; r|=r<<2; r|=r<<4;
      dstp[0]=r;
      dstp[1]=g;
      dstp[2]=b;
      dstp[3]=0xff;
    }
  }
  return DRIVER->fbcvt;
}

/* Convert and upload texture.
 */

static int fmn_macwm_upload_texture(struct video_driver *driver,const void *fb) {
  const void *glpixels=0;
  if (DRIVER->fbcvt) switch (driver->delegate.fbfmt) {
    case FMN_IMGFMT_thumby: glpixels=fmn_macwm_cvt_rgba_thumby(driver,fb); break;
    case FMN_IMGFMT_bgr565be: glpixels=fmn_macwm_cvt_rgba_bgr565be(driver,fb); break;
    case FMN_IMGFMT_bgr332: glpixels=fmn_macwm_cvt_rgba_bgr332(driver,fb); break;
  } else {
    glpixels=fb;
  }
  if (!glpixels) return -1;
  fmn_macwm_replace_fb(glpixels);
  return 0;
}

/* Swap.
 */

static int _macwm_swap(struct video_driver *driver,const void *fb) {
  if (fmn_macwm_upload_texture(driver,fb)<0) return -1;
  return 0;
}

/* Fullscreen.
 */

static int _macwm_fullscreen(struct video_driver *driver,int state) {
  if (state>0) {
    if (driver->fullscreen) return 1;
    if (fmn_macwm_toggle_fullscreen()<0) return -1;
    driver->fullscreen=1;
  } else if (!state) {
    if (!driver->fullscreen) return 0;
    if (fmn_macwm_toggle_fullscreen()<0) return -1;
    driver->fullscreen=0;
  } else {
    if (fmn_macwm_toggle_fullscreen()<0) return -1;
  }
  return driver->fullscreen;
}

/* Type definition.
 */

const struct video_driver_type video_driver_type_macwm={
  .name="macwm",
  .desc="MacOS window manager",
  .objlen=sizeof(struct video_driver_macwm),
  .del=_macwm_del,
  .init=_macwm_init,
  .swap=_macwm_swap,
  .fullscreen=_macwm_fullscreen,
};
