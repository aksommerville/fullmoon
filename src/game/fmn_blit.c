#include "fullmoon.h"
#include <string.h>

/* Blit one thumby image onto another.
 * Very likely for generating the background image, in which case it's also very likely to be byte-aligned.
 */
 
static void fmn_blit_thumby(
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

/* Blit a sprite from ya11 to a thumby framebuffer.
 * Probably the likeliest scenario and most deserving of optimization.
 */
 
static void fmn_blit_thumby_ya11(
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

/* Blit, main entry point.
 */
 
void fmn_blit(
  struct fmn_image *dst,int16_t dstx,int16_t dsty,
  const struct fmn_image *src,int16_t srcx,int16_t srcy,
  int16_t w,int16_t h
) {

  // Clip and validate.
  if (!dst||!src||!dst->writeable) return;
  if (dstx<0) { srcx-=dstx; w+=dstx; dstx=0; }
  if (dsty<0) { srcy-=dsty; h+=dsty; dsty=0; }
  if (srcx<0) { dstx-=srcx; w+=srcx; srcx=0; }
  if (srcy<0) { dsty-=srcy; h+=srcy; srcy=0; }
  if (dstx>dst->w-w) w=dst->w-dstx;
  if (dsty>dst->h-h) h=dst->h-dsty;
  if (srcx>src->w-w) w=src->w-srcx;
  if (srcy>src->h-h) h=src->h-srcy;
  if ((w<1)||(h<1)) return;
  
  // All likely format scenarios should have a bespoke blitter.
  switch ((dst->fmt<<8)|src->fmt) {
    case (FMN_IMGFMT_thumby<<8)|FMN_IMGFMT_thumby: fmn_blit_thumby(dst,dstx,dsty,src,srcx,srcy,w,h); return;
    case (FMN_IMGFMT_thumby<<8)|FMN_IMGFMT_ya11: fmn_blit_thumby_ya11(dst,dstx,dsty,src,srcx,srcy,w,h); return;
  }
  
  // In all other scenarios, we do it generically at terrible performance cost.
  //...or not at all (TODO)
}
