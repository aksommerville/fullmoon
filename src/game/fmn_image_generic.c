#include "fullmoon.h"

/* Format properties.
 */
 
uint32_t fmn_imgfmt_get_alpha_mask(uint8_t imgfmt) {
  switch (imgfmt) {
    case FMN_IMGFMT_thumby: return 0;
    case FMN_IMGFMT_ya11: return 1;
  }
  return 0;
}

/* Dummy iterator.
 */
 
static uint32_t fmn_pxrd_dummy(const uint8_t *p,uint8_t q) {
  return 0;
}

static void fmn_pxwr_dummy(uint8_t *p,uint8_t q,uint32_t src) {
}

static void fmn_next_dummy(struct fmn_image_iterator_1d *iter) {
}

/* Thumby iterator.
 * (q) is a one-bit mask.
 */
 
static uint32_t fmn_pxrd_thumby(const uint8_t *p,uint8_t q) {
  return ((*p)&q)?1:0;
}

static void fmn_pxwr_thumby(uint8_t *p,uint8_t q,uint32_t src) {
  if (src) (*p)|=q;
  else (*p)&=~q;
}

static void fmn_next_left_thumby(struct fmn_image_iterator_1d *iter) {
  iter->p--;
}

static void fmn_next_right_thumby(struct fmn_image_iterator_1d *iter) {
  iter->p++;
}

static void fmn_next_up_thumby(struct fmn_image_iterator_1d *iter) {
  if (iter->q==0x01) {
    iter->q=0x80;
    iter->p-=iter->stride;
  } else {
    iter->q>>=1;
  }
}

static void fmn_next_down_thumby(struct fmn_image_iterator_1d *iter) {
  if (iter->q==0x80) {
    iter->q=0x01;
    iter->p+=iter->stride;
  } else {
    iter->q<<=1;
  }
}
 
static void fmn_image_iterator_init_thumby(
  struct fmn_image_iterator *iter,
  int16_t x,int16_t y,
  int16_t dxminor,int16_t dyminor,
  int16_t dxmajor,int16_t dymajor
) {
  iter->major.p=iter->image->v+(y>>3)*iter->image->stride+x;
  iter->major.q=0x01<<(y&7);
  iter->read=fmn_pxrd_thumby;
  iter->write=fmn_pxwr_thumby;
       if (dxminor<0) iter->minor.next=fmn_next_left_thumby;
  else if (dxminor>0) iter->minor.next=fmn_next_right_thumby;
  else if (dyminor<0) iter->minor.next=fmn_next_up_thumby;
  else if (dyminor>0) iter->minor.next=fmn_next_down_thumby;
       if (dxmajor<0) iter->major.next=fmn_next_left_thumby;
  else if (dxmajor>0) iter->major.next=fmn_next_right_thumby;
  else if (dymajor<0) iter->major.next=fmn_next_up_thumby;
  else if (dymajor>0) iter->major.next=fmn_next_down_thumby;
}

/* ya11 iterator.
 * (q) is the shift: 0,2,4,6
 */
 
static uint32_t fmn_pxrd_ya11(const uint8_t *p,uint8_t q) {
  return ((*p)>>q)&3;
}

static void fmn_pxwr_ya11(uint8_t *p,uint8_t q,uint32_t src) {
  *p=((*p)&~(3<<q))|(src<<q);
}

static void fmn_next_left_ya11(struct fmn_image_iterator_1d *iter) {
  if (iter->q==6) {
    iter->q=0;
    iter->p--;
  } else {
    iter->q+=2;
  }
}

static void fmn_next_right_ya11(struct fmn_image_iterator_1d *iter) {
  if (iter->q==0) {
    iter->q=6;
    iter->p++;
  } else {
    iter->q-=2;
  }
}

static void fmn_next_up_ya11(struct fmn_image_iterator_1d *iter) {
  iter->p-=iter->stride;
}

static void fmn_next_down_ya11(struct fmn_image_iterator_1d *iter) {
  iter->p+=iter->stride;
}
 
static void fmn_image_iterator_init_ya11(
  struct fmn_image_iterator *iter,
  int16_t x,int16_t y,
  int16_t dxminor,int16_t dyminor,
  int16_t dxmajor,int16_t dymajor
) {
  iter->major.p=iter->image->v+y*iter->image->stride+(x>>2);
  iter->major.q=(3-(x&3))<<1;
  iter->read=fmn_pxrd_ya11;
  iter->write=fmn_pxwr_ya11;
       if (dxminor<0) iter->minor.next=fmn_next_left_ya11;
  else if (dxminor>0) iter->minor.next=fmn_next_right_ya11;
  else if (dyminor<0) iter->minor.next=fmn_next_up_ya11;
  else if (dyminor>0) iter->minor.next=fmn_next_down_ya11;
       if (dxmajor<0) iter->major.next=fmn_next_left_ya11;
  else if (dxmajor>0) iter->major.next=fmn_next_right_ya11;
  else if (dymajor<0) iter->major.next=fmn_next_up_ya11;
  else if (dymajor>0) iter->major.next=fmn_next_down_ya11;
}

/* Initialize iterator with digested transform.
 */
 
static void fmn_image_iterator_init(
  struct fmn_image_iterator *iter,
  int16_t x,int16_t y,int16_t w,int16_t h,
  int16_t dxminor,int16_t dyminor,
  int16_t dxmajor,int16_t dymajor
) {
  if (dxminor) {
    iter->minorc=w-1;
    iter->majorc=h-1;
  } else {
    iter->minorc=h-1;
    iter->majorc=w-1;
  }
  iter->minorc0=iter->minorc;
  iter->minor.stride=iter->image->stride;
  iter->major.stride=iter->image->stride;
  switch (iter->image->fmt) {
    /* Format-specific initializers must set:
     *   iter->read
     *   iter->write
     *   iter->major.p
     *   iter->major.q
     *   iter->major.next
     *   iter->minor.next
     * We figure out the rest.
     */
    case FMN_IMGFMT_thumby: fmn_image_iterator_init_thumby(iter,x,y,dxminor,dyminor,dxmajor,dymajor); break;
    case FMN_IMGFMT_ya11: fmn_image_iterator_init_ya11(iter,x,y,dxminor,dyminor,dxmajor,dymajor); break;
  }
  iter->minor.p=iter->major.p;
  iter->minor.q=iter->major.q;
}

/* Begin iteration.
 */
 
uint8_t fmn_image_iterate(
  struct fmn_image_iterator *iter,
  const struct fmn_image *image,
  int16_t x,int16_t y,int16_t w,int16_t h,
  uint8_t xform
) {
  if (!iter) return 0;
  iter->read=fmn_pxrd_dummy;
  iter->write=fmn_pxwr_dummy;
  if (!image) return 0;
  xform&=7;
  if (xform&FMN_XFORM_SWAP) {
    int16_t tmp=w;
    w=h;
    h=tmp;
  }
  if ((x<0)||(y<0)||(w<1)||(h<1)||(x>image->w-w)||(y>image->h-h)) return 0;
  iter->image=(void*)image;
  switch (xform) {
    #define X FMN_XFORM_XREV
    #define Y FMN_XFORM_YREV
    #define Z FMN_XFORM_SWAP
    case 0|0|0: fmn_image_iterator_init(iter,x    ,y    ,w,h, 1, 0, 0, 1); break;
    case X|0|0: fmn_image_iterator_init(iter,x+w-1,y    ,w,h,-1, 0, 0, 1); break;
    case 0|Y|0: fmn_image_iterator_init(iter,x    ,y+h-1,w,h, 1, 0, 0,-1); break;
    case X|Y|0: fmn_image_iterator_init(iter,x+w-1,y+h-1,w,h,-1, 0, 0,-1); break;
    case 0|0|Z: fmn_image_iterator_init(iter,x    ,y    ,w,h, 0, 1, 1, 0); break;
    case X|0|Z: fmn_image_iterator_init(iter,x+w-1,y    ,w,h, 0, 1,-1, 0); break;
    case 0|Y|Z: fmn_image_iterator_init(iter,x    ,y+h-1,w,h, 0,-1, 1, 0); break;
    case X|Y|Z: fmn_image_iterator_init(iter,x+w-1,y+h-1,w,h, 0,-1,-1, 0); break;
    #undef X
    #undef Y
    #undef Z
  }
  if (!iter->major.next||!iter->minor.next) {
    iter->major.next=fmn_next_dummy;
    iter->minor.next=fmn_next_dummy;
    return 0;
  }
  if (!image->writeable) iter->write=fmn_pxwr_dummy;
  return 1;
}

/* Step iterator.
 */
 
uint8_t fmn_image_iterator_next(struct fmn_image_iterator *iter) {
  if (iter->minorc) {
    iter->minor.next(&iter->minor);
    iter->minorc--;
    return 1;
  }
  if (iter->majorc) {
    iter->major.next(&iter->major);
    iter->majorc--;
    iter->minorc=iter->minorc0;
    iter->minor.p=iter->major.p;
    iter->minor.q=iter->major.q;
    return 1;
  }
  return 0;
}

/* Pixel conversion.
 * This is an anything-to-anything deal, so the count of functions we need is the square of the count of formats.
 * If we start adding lots of formats (why would we?), consider splitting these with a generic intermediate format.
 */
 
static uint32_t fmn_pixcvt_identity(uint32_t src) {
  return src;
}

static uint32_t fmn_pixcvt_thumby_ya11(uint32_t src) {
  return src>>1;
}

static uint32_t fmn_pixcvt_ya11_thumby(uint32_t src) {
  return (src<<1)|1;
}

fmn_pixcvt_fn fmn_pixcvt_get(uint8_t dstfmt,uint8_t srcfmt) {
  if (dstfmt==srcfmt) return fmn_pixcvt_identity;
  switch (dstfmt) {
    case FMN_IMGFMT_thumby: switch (srcfmt) {
        case FMN_IMGFMT_ya11: return fmn_pixcvt_thumby_ya11;
      } break;
    case FMN_IMGFMT_ya11: switch (srcfmt) {
        case FMN_IMGFMT_thumby: return fmn_pixcvt_ya11_thumby;
      } break;
  }
  return 0;
}
