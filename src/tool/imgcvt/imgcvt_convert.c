#include "imgcvt.h"

/* Iterate into thumby image.
 * Thumby image rows are naturally byte-aligned.
 */
 
struct imgcvt_fmn_iterator_thumby {
  uint8_t *p;
  uint8_t *p0;
  int yc;
  int xc;
  int xc0;
  uint8_t mask;
};

static int imgcvt_cb_thumby(uint8_t y,uint8_t a,void *userdata) {
  struct imgcvt_fmn_iterator_thumby *iter=userdata;
  
  if (y>=0x80) (*(iter->p))|=iter->mask;
  
  if (iter->xc>1) {
    iter->xc--;
    iter->p++;
  } else {
    if (iter->yc<2) return 1;
    iter->yc--;
    iter->xc=iter->xc0;
    if (iter->mask<0x80) {
      iter->mask<<=1;
    } else {
      iter->p0+=iter->xc0;
      if (iter->yc<8) iter->mask=(0x80>>(iter->yc-1));
      else iter->mask=0x01;
    }
    iter->p=iter->p0;
  }
  
  return 0;
}

/* Iterate into ya11 image.
 */
 
struct imgcvt_fmn_iterator_ya11 {
  uint8_t *p;
  uint8_t *p0;
  uint8_t shift;
  int xc;
  int xc0;
  int stride;
};

static int imgcvt_cb_ya11(uint8_t y,uint8_t a,void *userdata) {
  struct imgcvt_fmn_iterator_ya11 *iter=userdata;
  if ((y>=0x40)&&(y<0xc0)) a=0x00; // intermediate grays count as transparent
  
  uint8_t pixel=((y&0x80)>>6)|(a>>7);
  *(iter->p)|=(pixel<<iter->shift);
  
  if (iter->xc>1) {
    iter->xc--;
    if (iter->shift>0) iter->shift-=2;
    else {
      iter->shift=6;
      iter->p++;
    }
  } else {
    iter->xc=iter->xc0;
    iter->p0+=iter->stride;
    iter->p=iter->p0;
    iter->shift=6;
  }
  
  return 0;
}

/* Callback for alpha detector.
 */
 
static int imgcvt_cb_alpha(uint8_t y,uint8_t a,void *userdata) {
  if (a!=0xff) return 1;
  if ((y>=0x40)&&(y<0xc0)) return 1;
  return 0;
}

/* Convert to Fullmoon from PNG, main entry point.
 */
 
int imgcvt_fmn_from_png(struct imgcvt *imgcvt) {
  
  int alpha;
  if (imgcvt->png.depth==1) alpha=0;
  else alpha=png_image_iterate_ya88(&imgcvt->png,imgcvt_cb_alpha,0);
  
  imgcvt->image.writeable=0;
  imgcvt->image.w=imgcvt->png.w;
  imgcvt->image.h=imgcvt->png.h;
  
  if (alpha) {
    imgcvt->image.fmt=FMN_IMGFMT_ya11;
    imgcvt->image.stride=(imgcvt->image.w*2+7)>>3;
    if (!(imgcvt->image.v=calloc(imgcvt->image.stride,imgcvt->image.h))) return -1;
    struct imgcvt_fmn_iterator_ya11 iter={
      .p=imgcvt->image.v,
      .p0=imgcvt->image.v,
      .shift=6,
      .xc=imgcvt->image.w,
      .xc0=imgcvt->image.w,
      .stride=imgcvt->image.stride,
    };
    if (png_image_iterate_ya88(&imgcvt->png,imgcvt_cb_ya11,&iter)<0) return -1;
    
  } else {
    imgcvt->image.fmt=FMN_IMGFMT_thumby;
    imgcvt->image.stride=imgcvt->image.w;
    if (!(imgcvt->image.v=calloc(imgcvt->image.stride,((imgcvt->image.h+7)>>3)))) return -1;
    struct imgcvt_fmn_iterator_thumby iter={
      .p=imgcvt->image.v,
      .p0=imgcvt->image.v,
      .yc=imgcvt->image.h,
      .xc=imgcvt->image.w,
      .xc0=imgcvt->image.w,
      .mask=(imgcvt->image.h>=8)?0x01:(0x80>>(imgcvt->image.h-1)),
    };
    if (png_image_iterate_ya88(&imgcvt->png,imgcvt_cb_thumby,&iter)<0) return -1;
  }
  
  return 0;
}
