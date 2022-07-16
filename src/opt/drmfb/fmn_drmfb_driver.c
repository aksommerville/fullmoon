#include "fmn_drmfb_internal.h"

/* Cleanup.
 */
 
static void fmn_drmfb_fb_cleanup(struct video_driver *driver,struct fmn_drmfb_fb *fb) {
  if (fb->fbid) {
    drmModeRmFB(DRIVER->fd,fb->fbid);
  }
  if (fb->v) {
    munmap(fb->v,fb->size);
  }
}
 
static void _drmfb_del(struct video_driver *driver) {
  
  fmn_drmfb_fb_cleanup(driver,DRIVER->fbv+0);
  fmn_drmfb_fb_cleanup(driver,DRIVER->fbv+1);

  if (DRIVER->crtc_restore) {
    if (DRIVER->fd>=0) {
      drmModeCrtcPtr crtc=DRIVER->crtc_restore;
      drmModeSetCrtc(
        DRIVER->fd,
        crtc->crtc_id,
        crtc->buffer_id,
        crtc->x,
        crtc->y,
        &DRIVER->connid,
        1,
        &crtc->mode
      );
    }
    drmModeFreeCrtc(DRIVER->crtc_restore);
  }

  if (DRIVER->fd>=0) {
    close(DRIVER->fd);
    DRIVER->fd=-1;
  }
}

/* Init.
 */
 
static int _drmfb_init(struct video_driver *driver) {
  driver->fullscreen=1;
  if (fmn_drmfb_init_connection(driver)<0) return -1;
  if (fmn_drmfb_init_buffers(driver)<0) return -1;
  return 0;
}

/* Swap buffers.
 */
 
static int _drmfb_swap(struct video_driver *driver,const void *src) {
  
  DRIVER->fbp^=1;
  struct fmn_drmfb_fb *fb=DRIVER->fbv+DRIVER->fbp;
  
  fmn_drmfb_scale(fb,driver,src);
  
  while (1) {
    if (drmModePageFlip(DRIVER->fd,DRIVER->crtcid,fb->fbid,0,driver)<0) {
      if (errno==EBUSY) { // waiting for prior flip
        usleep(1000);
        continue;
      }
      fprintf(stderr,"drmModePageFlip: %m\n");
      return -1;
    } else {
      break;
    }
  }
  return 0;
}

/* Type definition.
 */
 
const struct video_driver_type video_driver_type_drmfb={
  .name="drmfb",
  .desc="Direct Rendering Manager for Linux, no acceleration.",
  .objlen=sizeof(struct video_driver_drmfb),
  .del=_drmfb_del,
  .init=_drmfb_init,
  .swap=_drmfb_swap,
};
