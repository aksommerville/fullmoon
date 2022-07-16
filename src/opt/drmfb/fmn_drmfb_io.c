#include "fmn_drmfb_internal.h"
#include <drm_fourcc.h>

/* Connector and mode chosen.
 */
 
static int fmn_drmfb_init_with_conn(
  struct video_driver *driver,
  drmModeResPtr res,
  drmModeConnectorPtr conn
) {

  /* TODO would be really cool if we could make this work with a "disconnected" monitor, with X running.
   * Understandably, a monitor that X is using will just not be available, I've made peace with that.
   * If I disable the larger monitor via Mate monitors control panel, we still detect it here and try to connect.
   * Its encoder_id is zero, but (conn->encoders) does list the right one.
   * No matter what I do though, when we reach the first drmModeSetCrtc it fails.
   * I'm guessing we have to tell DRM to use this encoder with this connection, but no idea how...
   */

  DRIVER->connid=conn->connector_id;
  DRIVER->encid=conn->encoder_id;
  DRIVER->screenw=DRIVER->mode.hdisplay;
  DRIVER->screenh=DRIVER->mode.vdisplay;
  
  drmModeEncoderPtr encoder=drmModeGetEncoder(DRIVER->fd,DRIVER->encid);
  if (!encoder) return -1;
  DRIVER->crtcid=encoder->crtc_id;
  drmModeFreeEncoder(encoder);
  
  // Store the current CRTC so we can restore it at quit.
  if (!(DRIVER->crtc_restore=drmModeGetCrtc(DRIVER->fd,DRIVER->crtcid))) return -1;

  return 0;
}

/* Filter and compare modes.
 */
 
static int fmn_drmfb_connector_acceptable(
  struct video_driver *driver,
  drmModeConnectorPtr conn
) {
  if (conn->connection!=DRM_MODE_CONNECTED) return 0;
  // Could enforce size limits against (mmWidth,mmHeight), probably not.
  return 1;
}

static int fmn_drmfb_mode_acceptable(
  struct video_driver *driver,
  drmModeConnectorPtr conn,
  drmModeModeInfoPtr mode
) {
  //XXX temporary hack to prevent 30 Hz output from the Atari
  if (mode->vrefresh<50) return 0;
  //if (mode->hdisplay<3000) return 0;
  return 1;
}

// <0=a, >0=b, 0=whatever.
// All connections and modes provided here have been deemed acceptable.
// (conna) and (connb) will often be the same object.
static int fmn_drmfb_mode_compare(
  struct video_driver *driver,
  drmModeConnectorPtr conna,drmModeModeInfoPtr modea,
  drmModeConnectorPtr connb,drmModeModeInfoPtr modeb
) {
  
  // Firstly, if just one has the "PREFERRED" flag, it's in play right now. Prefer that, so we don't switch resolution.
  // Two modes with this flag, eg two monitors attached, they cancel out and we look at other things.
  //TODO I guess there should be an option to override this. As is, it should always trigger.
  int prefera=(modea->type&DRM_MODE_TYPE_PREFERRED);
  int preferb=(modeb->type&DRM_MODE_TYPE_PREFERRED);
  if (prefera&&!preferb) return -1;
  if (!prefera&&preferb) return 1;
  
  // Our framebuffer is surely smaller than any available mode, so prefer the smaller mode.
  if ((modea->hdisplay!=modeb->hdisplay)||(modea->vdisplay!=modeb->vdisplay)) {
    int areaa=modea->hdisplay*modea->vdisplay;
    int areab=modeb->hdisplay*modeb->vdisplay;
    if (areaa<areab) return -1;
    if (areaa>areab) return 1;
  }
  // Hmm ok, closest to 60 Hz?
  if (modea->vrefresh!=modeb->vrefresh) {
    int da=modea->vrefresh-60; if (da<0) da=-da;
    int db=modeb->vrefresh-60; if (db<0) db=-db;
    int dd=da-db;
    if (dd<0) return -1;
    if (dd>0) return 1;
  }

  // If we get this far, the modes are actually identical, so whatever.
  return 0;
}

/* Choose the best connector and mode.
 * The returned connector is STRONG.
 * Mode selection is copied over (G.mode).
 */
 
static drmModeConnectorPtr fmn_drmfb_choose_connector(
  struct video_driver *driver,
  drmModeResPtr res
) {

  //XXX? Maybe just while developing...
  int log_all_modes=1;
  int log_selected_mode=1;

  if (log_all_modes) {
    fprintf(stderr,"%s:%d: %s...\n",__FILE__,__LINE__,__func__);
  }
  
  drmModeConnectorPtr best=0; // if populated, (DRIVER->mode) is also populated
  int conni=0;
  for (;conni<res->count_connectors;conni++) {
    drmModeConnectorPtr connq=drmModeGetConnector(DRIVER->fd,res->connectors[conni]);
    if (!connq) continue;
    
    if (log_all_modes) fprintf(stderr,
      "  CONNECTOR %d %s %dx%dmm type=%08x typeid=%08x\n",
      connq->connector_id,
      (connq->connection==DRM_MODE_CONNECTED)?"connected":"disconnected",
      connq->mmWidth,connq->mmHeight,
      connq->connector_type,
      connq->connector_type_id
    );
    
    if (!fmn_drmfb_connector_acceptable(driver,connq)) {
      drmModeFreeConnector(connq);
      continue;
    }
    
    drmModeModeInfoPtr modeq=connq->modes;
    int modei=connq->count_modes;
    for (;modei-->0;modeq++) {
    
      if (log_all_modes) fprintf(stderr,
        "    MODE %dx%d @ %d Hz, flags=%08x, type=%08x\n",
        modeq->hdisplay,modeq->vdisplay,
        modeq->vrefresh,
        modeq->flags,modeq->type
      );
    
      if (!fmn_drmfb_mode_acceptable(driver,connq,modeq)) continue;
      
      if (!best) {
        best=connq;
        DRIVER->mode=*modeq;
        continue;
      }
      
      if (fmn_drmfb_mode_compare(driver,best,&DRIVER->mode,connq,modeq)>0) {
        best=connq;
        DRIVER->mode=*modeq;
        continue;
      }
    }
    
    if (best!=connq) {
      drmModeFreeConnector(connq);
    }
  }
  if (best&&log_selected_mode) {
    fprintf(stderr,
      "drm: selected mode %dx%d @ %d Hz on connector %d\n",
      DRIVER->mode.hdisplay,DRIVER->mode.vdisplay,
      DRIVER->mode.vrefresh,
      best->connector_id
    );
  }
  return best;
}

/* File opened and resources acquired.
 */
 
static int fmn_drmfb_init_with_res(
  struct video_driver *driver,
  drmModeResPtr res
) {

  drmModeConnectorPtr conn=fmn_drmfb_choose_connector(driver,res);
  if (!conn) {
    fprintf(stderr,"drm: Failed to locate a suitable connector.\n");
    return -1;
  }
  
  int err=fmn_drmfb_init_with_conn(driver,res,conn);
  drmModeFreeConnector(conn);
  if (err<0) return -1;

  return 0;
}

/* Initialize connection.
 */
 
int fmn_drmfb_init_connection(struct video_driver *driver) {

  const char *path="/dev/dri/card0";//TODO Let client supply this, or scan /dev/dri/
  
  if ((DRIVER->fd=open(path,O_RDWR))<0) {
    fprintf(stderr,"%s: Failed to open DRM device: %m\n",path);
    return -1;
  }
  
  drmModeResPtr res=drmModeGetResources(DRIVER->fd);
  if (!res) {
    fprintf(stderr,"%s:drmModeGetResources: %m\n",path);
    return -1;
  }
  
  int err=fmn_drmfb_init_with_res(driver,res);
  drmModeFreeResources(res);
  if (err<0) return -1;
  
  return 0;
}

/* Given (fbw,fbh,screenw,screenh), calculate reasonable values for (scale,dstx,dsty,dstw,dsth).
 */
 
int fmn_drmfb_calculate_output_bounds(struct video_driver *driver) {

  const int overscan=32;
  int screenw=DRIVER->screenw-overscan;
  int screenh=DRIVER->screenh-overscan;
  
  // Use the largest legal scale factor.
  // Too small is very unlikely -- our input is 96x64. So we're not going to handle that case, just fail.
  int scalex=screenw/driver->delegate.fbw;
  int scaley=screenh/driver->delegate.fbh;
  DRIVER->scale=(scalex<scaley)?scalex:scaley;
  if (DRIVER->scale<1) {
    fprintf(stderr,
      "Unable to fit %dx%d framebuffer on this %dx%d screen.\n",
      driver->delegate.fbw,driver->delegate.fbh,DRIVER->screenw,DRIVER->screenh
    );
    return -1;
  }
  if (DRIVER->scale>FMN_DRMFB_SCALE_LIMIT) DRIVER->scale=FMN_DRMFB_SCALE_LIMIT;
  
  DRIVER->dstw=DRIVER->scale*driver->delegate.fbw;
  DRIVER->dsth=DRIVER->scale*driver->delegate.fbh;
  DRIVER->dstx=(DRIVER->screenw>>1)-(DRIVER->dstw>>1);
  DRIVER->dsty=(DRIVER->screenh>>1)-(DRIVER->dsth>>1);
  
  return 0;
}

/* Initialize one framebuffer.
 */
 
static int fmn_drmfb_fb_init(struct video_driver *driver,struct fmn_drmfb_fb *fb) {

  struct drm_mode_create_dumb creq={
    .width=DRIVER->screenw,
    .height=DRIVER->screenh,
    .bpp=32,
    .flags=0,
  };
  if (ioctl(DRIVER->fd,DRM_IOCTL_MODE_CREATE_DUMB,&creq)<0) {
    fprintf(stderr,"DRM_IOCTL_MODE_CREATE_DUMB: %m\n");
    return -1;
  }
  fb->handle=creq.handle;
  fb->size=creq.size;
  DRIVER->stridewords=creq.pitch;
  DRIVER->stridewords>>=2;
  fprintf(stderr,"creq.pitch=%d stridewords=%d\n",creq.pitch,DRIVER->stridewords);
  
  if (drmModeAddFB(
    DRIVER->fd,
    creq.width,creq.height,
    32,
    creq.bpp,creq.pitch,
    fb->handle,
    &fb->fbid
  )<0) {
    fprintf(stderr,"drmModeAddFB: %m\n");
    return -1;
  }
  
  struct drm_mode_map_dumb mreq={
    .handle=fb->handle,
  };
  if (ioctl(DRIVER->fd,DRM_IOCTL_MODE_MAP_DUMB,&mreq)<0) {
    fprintf(stderr,"DRM_IOCTL_MODE_MAP_DUMB: %m\n");
    return -1;
  }
  
  fb->v=mmap(0,fb->size,PROT_READ|PROT_WRITE,MAP_SHARED,DRIVER->fd,mreq.offset);
  if (fb->v==MAP_FAILED) {
    fprintf(stderr,"mmap: %m\n");
    return -1;
  }
  
  //XXX EXPERIMENTAL: Can we read the framebuffer's format?
  // #define DRM_IOCTL_MODE_GETFB2           DRM_IOWR(0xCE, struct drm_mode_fb_cmd2)
  #if 0
struct drm_mode_fb_cmd2 {
        __u32 fb_id;
        __u32 width;
        __u32 height;
        __u32 pixel_format; /* fourcc code from drm_fourcc.h */
        __u32 flags; /* see above flags */

        /*
         * In case of planar formats, this ioctl allows up to 4
         * buffer objects with offsets and pitches per plane.
         * The pitch and offset order is dictated by the fourcc,
         * e.g. NV12 (https://fourcc.org/yuv.php#NV12) is described as:
         *
         *   YUV 4:2:0 image with a plane of 8 bit Y samples
         *   followed by an interleaved U/V plane containing
         *   8 bit 2x2 subsampled colour difference samples.
         *
         * So it would consist of Y as offsets[0] and UV as
         * offsets[1].  Note that offsets[0] will generally
         * be 0 (but this is not required).
         *
         * To accommodate tiled, compressed, etc formats, a
         * modifier can be specified.  The default value of zero
         * indicates "native" format as specified by the fourcc.
         * Vendor specific modifier token.  Note that even though
         * it looks like we have a modifier per-plane, we in fact
         * do not. The modifier for each plane must be identical.
         * Thus all combinations of different data layouts for
         * multi plane formats must be enumerated as separate
         * modifiers.
         */
        __u32 handles[4];
        __u32 pitches[4]; /* pitch for each plane */
        __u32 offsets[4]; /* offset of each plane */
        __u64 modifier[4]; /* ie, tiling, compress */
};
  #endif
  struct drm_mode_fb_cmd2 cmd={
    .fb_id=fb->fbid,
  };
  if (ioctl(DRIVER->fd,DRM_IOCTL_MODE_GETFB2,&cmd)<0) {
    fprintf(stderr,"DRM_IOCTL_MODE_GETFB2: %m\n");
  } else {
    fprintf(stderr,"DRM_IOCTL_MODE_GETFB2 OK\n");
    fprintf(stderr,"  width: %d\n",cmd.width);
    fprintf(stderr,"  height: %d\n",cmd.height);
    fprintf(stderr,"  pixel_format: 0x%08x ",cmd.pixel_format);
    int shift=0; for (;shift<32;shift+=8) {
      char ch=cmd.pixel_format>>shift;
      if ((ch>=0x20)&&(ch<=0x7e)) fprintf(stderr,"%c",ch);
      else fprintf(stderr,"?");
    }
    fprintf(stderr,"\n");
  }
  //...hey it works! I got "AR24"
  
  return 0;
}

/* Record the output framebuffers' pixel format.
 * If we can't determine it, guess.
 */
 
static void fmn_drmfb_detect_fb_format(struct video_driver *driver) {
  // Assume AR24 if we don't hear otherwise.
  DRIVER->rshift=16;
  DRIVER->gshift=8;
  DRIVER->bshift=0;

  struct drm_mode_fb_cmd2 cmd={
    .fb_id=DRIVER->fbv[0].fbid,
  };
  if (ioctl(DRIVER->fd,DRM_IOCTL_MODE_GETFB2,&cmd)<0) {
    fprintf(stderr,"DRM_IOCTL_MODE_GETFB2 failed. Assuming that the screen uses XRGB8888.\n");
    return;
  }
  //TODO I'm not clear on what pixel_format means for byte order. On a big-endian host, are we supposed to swap these?
  switch (cmd.pixel_format) {
    case DRM_FORMAT_XRGB8888: DRIVER->rshift=16; DRIVER->gshift= 8; DRIVER->bshift= 0; break;
    case DRM_FORMAT_XBGR8888: DRIVER->rshift= 0; DRIVER->gshift= 8; DRIVER->bshift=16; break;
    case DRM_FORMAT_RGBX8888: DRIVER->rshift=24; DRIVER->gshift=16; DRIVER->bshift= 8; break;
    case DRM_FORMAT_BGRX8888: DRIVER->rshift= 8; DRIVER->gshift=16; DRIVER->bshift=24; break;
    case DRM_FORMAT_ARGB8888: DRIVER->rshift=16; DRIVER->gshift= 8; DRIVER->bshift= 0; break;
    case DRM_FORMAT_ABGR8888: DRIVER->rshift= 0; DRIVER->gshift= 8; DRIVER->bshift=16; break;
    case DRM_FORMAT_RGBA8888: DRIVER->rshift=24; DRIVER->gshift=16; DRIVER->bshift= 8; break;
    case DRM_FORMAT_BGRA8888: DRIVER->rshift= 8; DRIVER->gshift=16; DRIVER->bshift=24; break;
  }
}

/* Init buffers.
 */
 
int fmn_drmfb_init_buffers(struct video_driver *driver) {
  
  if (fmn_drmfb_fb_init(driver,DRIVER->fbv+0)<0) return -1;
  if (fmn_drmfb_fb_init(driver,DRIVER->fbv+1)<0) return -1;
  fmn_drmfb_detect_fb_format(driver);
  
  if (fmn_drmfb_calculate_output_bounds(driver)<0) return -1;
  
  if (drmModeSetCrtc(
    DRIVER->fd,
    DRIVER->crtcid,
    DRIVER->fbv[0].fbid,
    0,0,
    &DRIVER->connid,
    1,
    &DRIVER->mode
  )<0) {
    fprintf(stderr,"drmModeSetCrtc: %m\n");
    return -1;
  }
  
  return 0;
}
