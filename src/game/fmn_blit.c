#include "game/fullmoon.h"
#include <string.h>

/* Blit one thumby image onto another without xform.
 * Very likely for generating the background image, in which case it's also very likely to be byte-aligned.
 */
 
static void fmn_blit_thumby_NONE(
  struct fmn_image *dst,int16_t dstx,int16_t dsty,
  const struct fmn_image *src,int16_t srcx,int16_t srcy,
  int16_t w,int16_t h
) {
  
  // Optimize for the lucky and likely case that all vertical edges are byte-aligned.
  if (!(dsty&7)&&!(srcy&7)&&!(h&7)) {
    int16_t rowc=h>>3;
    uint8_t *dstrow=dst->v+(dsty>>3)*dst->stride+dstx;
    const uint8_t *srcrow=src->v+(srcy>>3)*src->stride+srcx;
    for (;rowc-->0;dstrow+=dst->stride,srcrow+=src->stride) {
      memcpy(dstrow,srcrow,w);
    }
    
  // Sub-optimal misaligned case.
  // This could be done better: Each output byte draws from 1 or 2 input bytes, but instead we're doing 8 independent bit copies.
  // I expect most misaligned blits will be ya11, so not going to bother figuring ^ that out, for now.
  } else {
    uint8_t *dstrow=dst->v+(dsty>>3)*dst->stride+dstx;
    uint8_t dstmask0=1<<(dsty&7);
    const uint8_t *srcrow=src->v+(srcy>>3)*src->stride+srcx;
    uint8_t srcmask0=1<<(srcy&7);
    for (;h-->0;) {
      uint8_t *dstp=dstrow;
      uint8_t dstmask=dstmask0;
      const uint8_t *srcp=srcrow;
      uint8_t srcmask=srcmask0;
      int16_t xi=w;
      for (;xi-->0;dstp++,srcp++) {
        if ((*srcp)&srcmask) (*dstp)|=dstmask;
        else (*dstp)&=~dstmask;
      }
      if (dstmask0==0x80) {
        dstmask0=0x01;
        dstrow+=dst->stride;
      } else dstmask0<<=1;
      if (srcmask0==0x80) {
        srcmask0=0x01;
        srcrow+=src->stride;
      } else srcmask0<<=1;
    }
  }
}

/* Blit a sprite from ya11 to a thumby framebuffer without xform.
 * Probably the likeliest scenario and most deserving of optimization.
 */
 
static void fmn_blit_thumby_ya11_NONE(
  struct fmn_image *dst,int16_t dstx,int16_t dsty,
  const struct fmn_image *src,int16_t srcx,int16_t srcy,
  int16_t w,int16_t h
) {
  uint8_t *dstrow=dst->v+(dsty>>3)*dst->stride+dstx;
  uint8_t dstmask0=1<<(dsty&7);
  const uint8_t *srcrow=src->v+srcy*src->stride+(srcx>>2);
  uint8_t srcmask0=0x80>>((srcx&3)<<1);
  for (;h-->0;srcrow+=src->stride) {
    uint8_t *dstp=dstrow;
    uint8_t dstmask=dstmask0;
    const uint8_t *srcp=srcrow;
    uint8_t srcmask=srcmask0;
    int16_t xi=w;
    for (;xi-->0;dstp++) {
    
      uint8_t luma=(*srcp)&srcmask;
      srcmask>>=1;
      uint8_t alpha=(*srcp)&srcmask;
      srcmask>>=1;
      if (!srcmask) {
        srcmask=0x80;
        srcp++;
      }
      
      if (alpha) {
        if (luma) (*dstp)|=dstmask;
        else (*dstp)&=~dstmask;
      }
    }
    if (dstmask0==0x80) {
      dstmask0=0x01;
      dstrow+=dst->stride;
    } else {
      dstmask0<<=1;
    }
  }
}

/* Blit a sprite from ya11 to a thumby framebuffer with xform.
 * All transforms are reasonably likely. Zero, the likeliest by far, is handled separate.
 */
 
static void fmn_blit_thumby_ya11_ANY(
  struct fmn_image *dst,int16_t dstx,int16_t dsty,
  const struct fmn_image *src,int16_t srcx,int16_t srcy,
  int16_t srcw,int16_t srch,
  uint8_t xform
) {

  // (dst) is the more complex space, so we'll iterate over that LRTB, and do the hard work in (src) space.
  uint8_t *dstrow=dst->v+(dsty>>3)*dst->stride+dstx;
  uint8_t dstmask0=1<<(dsty&7);
  int16_t dstw,dsth;
  if (xform&FMN_XFORM_SWAP) { dstw=srch; dsth=srcw; }
  else { dstw=srcw; dsth=srch; }
  
  const uint8_t *srcrow;
  uint8_t srcymask0;
  int8_t srcmaskdx=0,srcmaskdy=0;
  int16_t srcrowdx=0,srcrowdy=0;
  switch (xform) {
    #define SETUP(initx,inity,dxminor,dyminor,dxmajor,dymajor) { \
      srcrow=src->v+(srcy+(inity?(srch-1):0))*src->stride+((srcx+(initx?(srcw-1):0))>>2); \
      srcymask0=0x80>>(((srcx+(initx?(srcw-1):0))&3)<<1); \
      srcmaskdx=dxminor; \
      srcmaskdy=dxmajor; \
      srcrowdx=dyminor*src->stride; \
      srcrowdy=dymajor*src->stride; \
    }
    case 0: SETUP(0,0,1,0,0,1) break;
    case FMN_XFORM_XREV: SETUP(1,0,-1,0,0,1) break;
    case FMN_XFORM_YREV: SETUP(0,1,1,0,0,-1) break;
    case FMN_XFORM_XREV|FMN_XFORM_YREV: SETUP(1,1,-1,0,0,-1) break;
    case FMN_XFORM_SWAP: SETUP(0,0,0,1,1,0) break;
    case FMN_XFORM_SWAP|FMN_XFORM_XREV: SETUP(1,0,0,1,-1,0) break;
    case FMN_XFORM_SWAP|FMN_XFORM_YREV: SETUP(0,1,0,-1,1,0) break;
    case FMN_XFORM_SWAP|FMN_XFORM_XREV|FMN_XFORM_YREV: SETUP(1,1,0,-1,-1,0) break;
    #undef SETUP
    default: return;
  }
  uint8_t srcamask0=srcymask0>>1;
  
  for (;dsth-->0;) {
    uint8_t *dstp=dstrow;
    uint8_t dstmask=dstmask0;
    const uint8_t *srcp=srcrow;
    uint8_t srcymask=srcymask0;
    uint8_t srcamask=srcamask0;
    int16_t xi=dstw;
    for (;xi-->0;dstp++) {
    
      uint8_t luma=(*srcp)&srcymask;
      uint8_t alpha=(*srcp)&srcamask;
      if (srcmaskdx<0) {
        if (srcymask==0x80) {
          srcymask=0x02;
          srcamask=0x01;
          srcp--;
        } else {
          srcymask<<=2;
          srcamask<<=2;
        }
      } else if (srcmaskdx>0) {
        if (srcymask==0x02) {
          srcymask=0x80;
          srcamask=0x40;
          srcp++;
        } else {
          srcymask>>=2;
          srcamask>>=2;
        }
      }
      srcp+=srcrowdx;
      
      if (alpha) {
        if (luma) (*dstp)|=dstmask;
        else (*dstp)&=~dstmask;
      }
    }
    if (dstmask0==0x80) {
      dstmask0=0x01;
      dstrow+=dst->stride;
    } else {
      dstmask0<<=1;
    }
    
    if (srcmaskdy<0) {
      if (srcymask==0x80) {
        srcymask0=0x02;
        srcamask0=0x01;
        srcrow--;
      } else {
        srcymask0<<=2;
        srcamask0<<=2;
      }
    } else if (srcmaskdy>0) {
      if (srcymask0==0x02) {
        srcymask0=0x80;
        srcamask0=0x40;
        srcrow++;
      } else {
        srcymask0>>=2;
        srcamask0>>=2;
      }
    }
    srcrow+=srcrowdy;
  }
}

/* Blit, main entry point.
 */
 
void fmn_blit(
  struct fmn_image *dst,int16_t dstx,int16_t dsty,
  const struct fmn_image *src,int16_t srcx,int16_t srcy,
  int16_t w,int16_t h,
  uint8_t xform
) {

  // Clip and validate.
  // The xform bits are orthogonal and could be examined individually, but it bends my brain when SWAP is involved.
  // So whatever, 8 cases, one for each complete xform.
  if (!dst||!src||!dst->writeable) return;
  switch (xform) {
    case 0: {
        if (dstx<0) { srcx-=dstx; w+=dstx; dstx=0; }
        if (srcx<0) { dstx-=srcx; w+=srcx; srcx=0; }
        if (dstx>dst->w-w) w=dst->w-dstx;
        if (srcx>src->w-w) w=src->w-srcx;
        if (dsty<0) { srcy-=dsty; h+=dsty; dsty=0; }
        if (srcy<0) { dsty-=srcy; h+=srcy; srcy=0; }
        if (dsty>dst->h-h) h=dst->h-dsty;
        if (srcy>src->h-h) h=src->h-srcy;
      } break;
    case FMN_XFORM_XREV: {
        if (dstx<0) { w+=dstx; dstx=0; }
        if (srcx<0) { w+=srcx; srcx=0; }
        if (dstx>dst->w-w) { srcx+=dstx+w-dst->w; w=dst->w-dstx; }
        if (srcx>src->w-w) { dstx+=srcx+w-src->w; w=src->w-srcx; }
        if (dsty<0) { srcy-=dsty; h+=dsty; dsty=0; }
        if (srcy<0) { dsty-=srcy; h+=srcy; srcy=0; }
        if (dsty>dst->h-h) h=dst->h-dsty;
        if (srcy>src->h-h) h=src->h-srcy;
      } break;
    case FMN_XFORM_YREV: {
        if (dstx<0) { srcx-=dstx; w+=dstx; dstx=0; }
        if (srcx<0) { dstx-=srcx; w+=srcx; srcx=0; }
        if (dstx>dst->w-w) w=dst->w-dstx;
        if (srcx>src->w-w) w=src->w-srcx;
        if (dsty<0) { h+=dsty; dsty=0; }
        if (srcy<0) { h+=srcy; srcy=0; }
        if (dsty>dst->h-h) { srcy+=dsty+h-dst->h; h=dst->h-dsty; }
        if (srcy>src->h-h) { dsty+=srcy+h-src->h; h=src->h-srcy; }
      } break;
    case FMN_XFORM_XREV|FMN_XFORM_YREV: {
        if (dstx<0) { w+=dstx; dstx=0; }
        if (srcx<0) { w+=srcx; srcx=0; }
        if (dstx>dst->w-w) { srcx+=dstx+w-dst->w; w=dst->w-dstx; }
        if (srcx>src->w-w) { dstx+=srcx+w-src->w; w=src->w-srcx; }
        if (dsty<0) { h+=dsty; dsty=0; }
        if (srcy<0) { h+=srcy; srcy=0; }
        if (dsty>dst->h-h) { srcy+=dsty+h-dst->h; h=dst->h-dsty; }
        if (srcy>src->h-h) { dsty+=srcy+h-src->h; h=src->h-srcy; }
      } break;
    case FMN_XFORM_SWAP: {
        if (dstx<0) { srcy-=dstx; h+=dstx; dstx=0; }
        if (srcx<0) { dsty-=srcx; w+=srcx; srcx=0; }
        if (dstx>dst->w-h) h=dst->w-dstx;
        if (srcx>src->w-w) w=src->w-srcx;
        if (dsty<0) { srcx-=dsty; w+=dsty; dsty=0; }
        if (srcy<0) { dstx-=srcy; h+=srcy; srcy=0; }
        if (dsty>dst->h-w) w=dst->h-dsty;
        if (srcy>src->h-h) h=src->h-srcy;
      } break;
    case FMN_XFORM_SWAP|FMN_XFORM_XREV: {
        if (dstx<0) { srcy-=dstx; h+=dstx; dstx=0; }
        if (srcx<0) { w+=srcx; srcx=0; }
        if (dstx>dst->w-h) h=dst->w-dstx;
        if (srcx>src->w-w) { dsty+=srcx+w-src->w; w=src->w-srcx; }
        if (dsty<0) { w+=dsty; dsty=0; }
        if (srcy<0) { dstx-=srcy; h+=srcy; srcy=0; }
        if (dsty>dst->h-w) { srcx+=dsty+w-dst->h; w=dst->h-dsty; }
        if (srcy>src->h-h) h=src->h-srcy;
      } break;
    case FMN_XFORM_SWAP|FMN_XFORM_YREV: {
        if (dstx<0) { h+=dstx; dstx=0; }
        if (srcx<0) { dsty-=srcx; w+=srcx; srcx=0; }
        if (dstx>dst->w-h) { srcy+=dstx+h-dst->w; h=dst->w-dstx; }
        if (srcx>src->w-w) w=src->w-srcx;
        if (dsty<0) { srcx-=dsty; w+=dsty; dsty=0; }
        if (srcy<0) { h+=srcy; srcy=0; }
        if (dsty>dst->h-w) w=dst->h-dsty;
        if (srcy>src->h-h) { dstx+=srcy+h-src->h; h=src->h-srcy; }
      } break;
    case FMN_XFORM_SWAP|FMN_XFORM_XREV|FMN_XFORM_YREV: {
        if (dstx<0) { h+=dstx; dstx=0; }
        if (srcx<0) { w+=srcx; srcx=0; }
        if (dstx>dst->w-h) { srcy+=dstx+h-dst->w; h=dst->w-dstx; }
        if (srcx>src->w-w) { dsty+=srcx+w-src->w; w=src->w-srcx; }
        if (dsty<0) { w+=dsty; dsty=0; }
        if (srcy<0) { h+=srcy; srcy=0; }
        if (dsty>dst->h-w) { srcx+=dsty+w-dst->h; w=dst->h-dsty; }
        if (srcy>src->h-h) { dstx+=srcy+h-src->h; h=src->h-srcy; }
      } break;
    default: return;
  }
  if ((w<1)||(h<1)) return;
  
  // All likely format scenarios should have a bespoke blitter.
  switch ((xform<<16)|(dst->fmt<<8)|src->fmt) {
    case (0<<16)|(FMN_IMGFMT_thumby<<8)|FMN_IMGFMT_thumby: fmn_blit_thumby_NONE(dst,dstx,dsty,src,srcx,srcy,w,h); return;
    case (0<<16)|(FMN_IMGFMT_thumby<<8)|FMN_IMGFMT_ya11: fmn_blit_thumby_ya11_NONE(dst,dstx,dsty,src,srcx,srcy,w,h); return;
  }
  if ((dst->fmt==FMN_IMGFMT_thumby)&&(src->fmt==FMN_IMGFMT_ya11)) {
    fmn_blit_thumby_ya11_ANY(dst,dstx,dsty,src,srcx,srcy,w,h,xform);
    return;
  }
  
  // In all other scenarios, we do it generically at terrible performance cost.
  struct fmn_image_iterator srciter={0},dstiter={0};
  if (!fmn_image_iterate(&srciter,src,srcx,srcy,w,h,xform&(FMN_XFORM_XREV|FMN_XFORM_YREV))) return;
  if (!fmn_image_iterate(&dstiter,dst,dstx,dsty,w,h,xform&FMN_XFORM_SWAP)) return;
  uint32_t alpha=fmn_imgfmt_get_alpha_mask(src->fmt);
  if (src->fmt==dst->fmt) {
    if (alpha) {
      do {
        uint32_t pixel=fmn_image_iterator_read(&srciter);
        if (pixel&alpha) {
          fmn_image_iterator_write(&dstiter,pixel);
        }
      } while (fmn_image_iterator_next(&dstiter)&&fmn_image_iterator_next(&srciter));
    } else {
      do {
        fmn_image_iterator_write(&dstiter,fmn_image_iterator_read(&srciter));
      } while (fmn_image_iterator_next(&dstiter)&&fmn_image_iterator_next(&srciter));
    }
  } else {
    fmn_pixcvt_fn pixcvt=fmn_pixcvt_get(dst->fmt,src->fmt);
    if (!pixcvt) return;
    if (alpha) {
      do {
        uint32_t pixel=fmn_image_iterator_read(&srciter);
        if (pixel&alpha) {
          fmn_image_iterator_write(&dstiter,pixcvt(pixel));
        }
      } while (fmn_image_iterator_next(&dstiter)&&fmn_image_iterator_next(&srciter));
    } else {
      do {
        fmn_image_iterator_write(&dstiter,pixcvt(fmn_image_iterator_read(&srciter)));
      } while (fmn_image_iterator_next(&dstiter)&&fmn_image_iterator_next(&srciter));
    }
  }
}

/* Fill rect.
 */

void fmn_image_fill_rect(
  struct fmn_image *dst,
  int16_t x,int16_t y,int16_t w,int16_t h,
  uint32_t pixel
) {
  if (!dst||!dst->writeable) return;
  if (x<0) { w+=x; x=0; }
  if (y<0) { h+=y; y=0; }
  if (x>dst->w-w) w=dst->w-x;
  if (y>dst->h-h) h=dst->h-y;
  if ((w<1)||(h<1)) return;
  //TODO I'm only using this for troubleshooting. If it comes up in real life, optimize for specific formats.
  struct fmn_image_iterator iter={0};
  if (!fmn_image_iterate(&iter,dst,x,y,w,h,0)) return;
  do {
    fmn_image_iterator_write(&iter,pixel);
  } while (fmn_image_iterator_next(&iter));
}
