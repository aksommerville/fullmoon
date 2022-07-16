#include "fmn_drmfb_internal.h"

/* From bgr565be.
 */
 
static void fmn_drmfb_scale_bgr565be(struct fmn_drmfb_fb *dst,struct video_driver *driver,const void *src) {
  uint32_t *dstrow=dst->v;
  int dststride=DRIVER->stridewords;
  dstrow+=DRIVER->dsty*dststride+DRIVER->dstx;
  int cpc=DRIVER->dstw<<2;
  const uint8_t *srcrow=src;
  int srcstride=driver->delegate.fbw<<1;
  int yi=driver->delegate.fbh;
  for (;yi-->0;srcrow+=srcstride) {
    uint32_t *dstp=dstrow;
    const uint8_t *srcp=srcrow;
    int xi=driver->delegate.fbw;
    for (;xi-->0;srcp+=2) {
      uint8_t r=srcp[1]<<3; r|=r>>5;
      uint8_t g=(srcp[0]<<5)|((srcp[1]>>3)&0x1c); g|=g>>6;
      uint8_t b=srcp[0]&0xf8; b|=b>>5;
      uint32_t pixel=(r<<DRIVER->rshift)|(g<<DRIVER->gshift)|(b<<DRIVER->bshift);
      int ri=DRIVER->scale;
      for (;ri-->0;dstp++) *dstp=pixel;
    }
    const void *dstrow0=dstrow;
    dstrow+=dststride;
    int ri=DRIVER->scale-1;
    for (;ri-->0;dstrow+=dststride) memcpy(dstrow,dstrow0,cpc);
  }
}

/* From rgba8888.
 */
 
static void fmn_drmfb_scale_rgba8888(struct fmn_drmfb_fb *dst,struct video_driver *driver,const void *src) {
  uint32_t *dstrow=dst->v;
  int dststride=DRIVER->stridewords;
  dstrow+=DRIVER->dsty*dststride+DRIVER->dstx;
  int cpc=DRIVER->dstw<<2;
  const uint8_t *srcrow=src;
  int srcstride=driver->delegate.fbw<<2;
  int yi=driver->delegate.fbh;
  for (;yi-->0;srcrow+=srcstride) {
    uint32_t *dstp=dstrow;
    const uint8_t *srcp=srcrow;
    int xi=driver->delegate.fbw;
    for (;xi-->0;srcp+=4) {
      uint32_t pixel=(srcp[0]<<DRIVER->rshift)|(srcp[1]<<DRIVER->gshift)|(srcp[2]<<DRIVER->bshift);
      int ri=DRIVER->scale;
      for (;ri-->0;dstp++) *dstp=pixel;
    }
    const void *dstrow0=dstrow;
    dstrow+=dststride;
    int ri=DRIVER->scale-1;
    for (;ri-->0;dstrow+=dststride) memcpy(dstrow,dstrow0,cpc);
  }
}

/* From xrgbn.
 */
 
static void fmn_drmfb_scale_xrgbn(struct fmn_drmfb_fb *dst,struct video_driver *driver,const void *src) {
  uint32_t *dstrow=dst->v;
  int dststride=DRIVER->stridewords;
  dstrow+=DRIVER->dsty*dststride+DRIVER->dstx;
  int cpc=DRIVER->dstw<<2;
  const uint32_t *srcrow=src;
  int srcstride=driver->delegate.fbw;
  int yi=driver->delegate.fbh;
  
  // There's a good chance that XRGBN is the format of the intermediate buffer. If so, we can avoid a lot of bit-twiddling.
  if ((DRIVER->rshift==16)&&(DRIVER->gshift==8)&&(DRIVER->bshift==0)) {
    for (;yi-->0;srcrow+=srcstride) {
      uint32_t *dstp=dstrow;
      const uint32_t *srcp=srcrow;
      int xi=driver->delegate.fbw;
      for (;xi-->0;srcp++) {
        int ri=DRIVER->scale;
        for (;ri-->0;dstp++) *dstp=*srcp;
      }
      const void *dstrow0=dstrow;
      dstrow+=dststride;
      int ri=DRIVER->scale-1;
      for (;ri-->0;dstrow+=dststride) memcpy(dstrow,dstrow0,cpc);
    }
    
  } else { // The generic case isn't too bad either:
    for (;yi-->0;srcrow+=srcstride) {
      uint32_t *dstp=dstrow;
      const uint32_t *srcp=srcrow;
      int xi=driver->delegate.fbw;
      for (;xi-->0;srcp++) {
        uint8_t r=(*srcp)>>16,g=(*srcp)>>8,b=(*srcp);
        uint32_t pixel=(r<<DRIVER->rshift)|(g<<DRIVER->gshift)|(b<<DRIVER->bshift);
        int ri=DRIVER->scale;
        for (;ri-->0;dstp++) *dstp=pixel;
      }
      const void *dstrow0=dstrow;
      dstrow+=dststride;
      int ri=DRIVER->scale-1;
      for (;ri-->0;dstrow+=dststride) memcpy(dstrow,dstrow0,cpc);
    }
  }
}

/* From Thumby.
 */
 
static void fmn_drmfb_scale_thumby(struct fmn_drmfb_fb *dst,struct video_driver *driver,const void *src) {

  // We are going to assume that the dimensions land on byte boundaries, because they do.
  if ((driver->delegate.fbw&7)||(driver->delegate.fbh&7)) return;
  
  uint32_t *dstrow=dst->v;
  int dststride=DRIVER->stridewords;
  dstrow+=DRIVER->dsty*dststride+DRIVER->dstx;
  int cpc=DRIVER->dstw<<2;
  const uint8_t *srcrow=src;
  int srcstride=driver->delegate.fbw; // sic, reading rowwise, each pixel is one byte...
  uint8_t srcmask=0x01; // ...but we read just one bit from it
  int yi=driver->delegate.fbh;
  for (;yi-->0;) {
    uint32_t *dstp=dstrow;
    const uint8_t *srcp=srcrow;
    int xi=driver->delegate.fbw;
    for (;xi-->0;srcp++) {
      uint32_t pixel=((*srcp)&srcmask)?0xffffffff:0x00000000;
      int ri=DRIVER->scale;
      for (;ri-->0;dstp++) *dstp=pixel;
    }
    const void *dstrow0=dstrow;
    dstrow+=dststride;
    int ri=DRIVER->scale-1;
    for (;ri-->0;dstrow+=dststride) memcpy(dstrow,dstrow0,cpc);
    srcmask<<=1;
    if (!srcmask) {
      srcmask=0x01;
      srcrow+=srcstride;
    }
  }
}

/* Scale.
 */
 
void fmn_drmfb_scale(struct fmn_drmfb_fb *dst,struct video_driver *driver,const void *src) {
  switch (driver->delegate.fbfmt) {
    case FMN_IMGFMT_bgr565be: fmn_drmfb_scale_bgr565be(dst,driver,src); return;
    case FMN_IMGFMT_rgba8888: fmn_drmfb_scale_rgba8888(dst,driver,src); return;
    //case FMN_IMGFMT_xrgbn: fmn_drmfb_scale_xrgbn(dst,driver,src); return;
    case FMN_IMGFMT_thumby: fmn_drmfb_scale_thumby(dst,driver,src); return;
  }
}
