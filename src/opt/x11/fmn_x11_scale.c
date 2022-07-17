#include "fmn_x11_internal.h"

/* From bgr565be.
 */
 
static void fmn_x11_scale_bgr565be(struct video_driver *driver,const void *src) {
  uint32_t *dstrow=(void*)DRIVER->image->data;
  int dststride=DRIVER->image->width;
  int cpc=dststride<<2;
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
    int ri=DRIVER->scale-1;
    for (;ri-->0;dstp+=DRIVER->image->width) memcpy(dstp,dstrow,cpc);
    dstrow=dstp;
  }
}

/* From rgba8888.
 */
 
static void fmn_x11_scale_rgba8888(struct video_driver *driver,const void *src) {
  uint32_t *dstrow=(void*)DRIVER->image->data;
  int dststride=DRIVER->image->width;
  int cpc=dststride<<2;
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
    int ri=DRIVER->scale-1;
    for (;ri-->0;dstp+=DRIVER->image->width) memcpy(dstp,dstrow,cpc);
    dstrow=dstp;
  }
}

/* From xrgbn.
 */
 
static void fmn_x11_scale_xrgbn(struct video_driver *driver,const void *src) {
  uint32_t *dstrow=(void*)DRIVER->image->data;
  int dststride=DRIVER->image->width;
  int cpc=dststride<<2;
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
      int ri=DRIVER->scale-1;
      for (;ri-->0;dstp+=DRIVER->image->width) memcpy(dstp,dstrow,cpc);
      dstrow=dstp;
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
      int ri=DRIVER->scale-1;
      for (;ri-->0;dstp+=dststride) memcpy(dstp,dstrow,cpc);
      dstrow=dstp;
    }
  }
}

/* From thumby, unique y1-ish format.
 */
 
static void fmn_x11_scale_thumby(struct video_driver *driver,const void *src) {

  // We are going to assume that the dimensions land on byte boundaries, because they do.
  if ((driver->delegate.fbw&7)||(driver->delegate.fbh&7)) return;
  
  uint32_t white=0xffffffff;
  uint32_t black=0x00000000;
  
  uint32_t *dstrow=(void*)DRIVER->image->data;
  int dststride=DRIVER->image->width;
  int cpc=dststride<<2;
  const uint8_t *srcrow=src;
  int srcstride=driver->delegate.fbw; // sic, reading rowwise, each pixel is one byte...
  uint8_t srcmask=0x01; // ...but we read just one bit from it
  int yi=driver->delegate.fbh;
  for (;yi-->0;) {
    uint32_t *dstp=dstrow;
    const uint8_t *srcp=srcrow;
    int xi=driver->delegate.fbw;
    for (;xi-->0;srcp++) {
      uint32_t pixel=((*srcp)&srcmask)?white:black;
      int ri=DRIVER->scale;
      for (;ri-->0;dstp++) *dstp=pixel;
    }
    int ri=DRIVER->scale-1;
    for (;ri-->0;dstp+=dststride) memcpy(dstp,dstrow,cpc);
    dstrow=dstp;
    srcmask<<=1;
    if (!srcmask) {
      srcmask=0x01;
      srcrow+=srcstride;
    }
  }
}

/* From regular big-endian y1.
 */
 
static void fmn_x11_scale_y1(struct video_driver *driver,const void *src) {
  uint32_t *dstrow=(void*)DRIVER->image->data;
  int dststride=DRIVER->image->width;
  int cpc=dststride<<2;
  const uint8_t *srcrow=src;
  int srcstride=(driver->delegate.fbw+7)>>3;
  int yi=driver->delegate.fbh;
  for (;yi-->0;srcrow+=srcstride) {
    uint32_t *dstp=dstrow;
    const uint8_t *srcp=srcrow;
    uint8_t srcmask=0x80;
    int xi=driver->delegate.fbw;
    for (;xi-->0;) {
      uint32_t pixel=((*srcp)&srcmask)?0xffffffff:0x00000000;
      if (!(srcmask>>=1)) {
        srcmask=0x80;
        srcp++;
      }
      int ri=DRIVER->scale;
      for (;ri-->0;dstp++) *dstp=pixel;
    }
    int ri=DRIVER->scale-1;
    for (;ri-->0;dstp+=dststride) memcpy(dstp,dstrow,cpc);
    dstrow=dstp;
  }
}

/* Scale framebuffer into our intermediate image, dispatch on format.
 */
 
void fmn_x11_scale(struct video_driver *driver,const void *src) {
  switch (driver->delegate.fbfmt) {
    case FMN_IMGFMT_bgr565be: fmn_x11_scale_bgr565be(driver,src); return;
    case FMN_IMGFMT_rgba8888: fmn_x11_scale_rgba8888(driver,src); return;
    //case FMN_IMGFMT_xrgbn: fmn_x11_scale_xrgbn(driver,src); return;
    case FMN_IMGFMT_thumby: fmn_x11_scale_thumby(driver,src); return;
    case FMN_IMGFMT_y1: fmn_x11_scale_y1(driver,src); return;
  }
}
