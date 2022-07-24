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
  GLuint texid;
  GLfloat dstl,dstt,dstr,dstb; // in -1..1, ie "device coordinates"
  int winw,winh; // macwm may change our (w,h) behind our back; use these to detect it
  uint8_t *y8;
};

#define DRIVER ((struct video_driver_macwm*)driver)

/* Cleanup.
 */

static void _macwm_del(struct video_driver *driver) {
  if (DRIVER->texid) glDeleteTextures(1,&DRIVER->texid);
  if (DRIVER->y8) free(DRIVER->y8);
  fmn_macwm_quit();
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

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1,&DRIVER->texid);
  if (!DRIVER->texid) {
    glGenTextures(1,&DRIVER->texid);
    if (!DRIVER->texid) return -1;
  }
  glBindTexture(GL_TEXTURE_2D,DRIVER->texid);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

  if (!(DRIVER->y8=malloc(driver->delegate.fbw*driver->delegate.fbh))) return -1;
  if (driver->delegate.fbfmt!=FMN_IMGFMT_thumby) {
    fprintf(stderr,"%s: Expected 'thumby' framebuffer format (%d), found %d.\n",__FILE__,FMN_IMGFMT_thumby,driver->delegate.fbfmt);
    return -1;
  }

  // Leave (winw,winh) unset; we'll notice at the first swap.

  return 0;
}

/* Recalculate output position if needed.
 */

static int fmn_macwm_require_bounds(struct video_driver *driver) {
  if ((DRIVER->winw==driver->w)&&(DRIVER->winh==driver->h)) return 0;
  DRIVER->winw=driver->w;
  DRIVER->winh=driver->h;
  if (!driver->w||!driver->h) return 0;

  // If we're this close to the window size, use the whole window and let it stretch a little.
  const GLfloat fudge=0.90f;

  GLfloat fbaspect=(GLfloat)driver->delegate.fbw/(GLfloat)driver->delegate.fbh;
  GLfloat winaspect=(GLfloat)driver->w/(GLfloat)driver->h;
  if (winaspect>fbaspect) {
    DRIVER->dstt=1.0f;
    DRIVER->dstr=fbaspect/winaspect;
    if (DRIVER->dstr>=fudge) DRIVER->dstr=1.0f;
  } else {
    DRIVER->dstr=1.0f;
    DRIVER->dstt=winaspect/fbaspect;
    if (DRIVER->dstb>=fudge) DRIVER->dstb=1.0f;
  }
  
  // Always centered:
  DRIVER->dstl=-DRIVER->dstr;
  DRIVER->dstb=-DRIVER->dstt;
  return 0;
}

/* Convert and upload texture.
 */

static int fmn_macwm_upload_texture(struct video_driver *driver,const void *fb) {

  const uint8_t *srcrow=fb;
  uint8_t srcmask=0x01;
  uint8_t *dstp=DRIVER->y8;
  int yi=driver->delegate.fbh;
  for (;yi-->0;) {
    const uint8_t *srcp=srcrow;
    int xi=driver->delegate.fbw;
    for (;xi-->0;srcp++,dstp++) {
      *dstp=((*srcp)&srcmask)?0xff:0x00;
    }
    if (srcmask==0x80) {
      srcmask=0x01;
      srcrow+=driver->delegate.fbw;
    } else {
      srcmask<<=1;
    }
  }

  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,driver->delegate.fbw,driver->delegate.fbh,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,DRIVER->y8);

  return 0;
}

/* Swap.
 */

static int _macwm_swap(struct video_driver *driver,const void *fb) {
  if (fmn_macwm_require_bounds(driver)<0) return -1;
  
  if (fmn_macwm_begin_video()<0) return -1;

  glViewport(0,0,driver->w,driver->h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if (
    (DRIVER->dstl>-1.0f)||(DRIVER->dstr<1.0f)||
    (DRIVER->dstt>-1.0f)||(DRIVER->dstb<1.0f)
  ) {
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  if (fmn_macwm_upload_texture(driver,fb)<0) return -1;

  glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2i(0,0); glVertex2f(DRIVER->dstl,DRIVER->dstt);
    glTexCoord2i(0,1); glVertex2f(DRIVER->dstl,DRIVER->dstb);
    glTexCoord2i(1,0); glVertex2f(DRIVER->dstr,DRIVER->dstt);
    glTexCoord2i(1,1); glVertex2f(DRIVER->dstr,DRIVER->dstb);
  glEnd();
  
  if (fmn_macwm_end_video()<0) return -1;
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
