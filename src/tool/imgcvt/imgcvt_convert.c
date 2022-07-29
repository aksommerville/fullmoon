#include "imgcvt.h"

/* Set (imgcvt->format) based on (imgcvt->png).
 */
 
static int imgcvt_cb_alpha(uint8_t y,uint8_t a,void *userdata) {
  if (a<0x80) return 1;
  if ((y>=0x40)&&(y<0xc0)) return 1;
  return 0;
}
 
static int imgcvt_guess_format(struct imgcvt *imgcvt) {
  int alpha=png_image_iterate_ya88(&imgcvt->png,imgcvt_cb_alpha,0);
  imgcvt->image.alpha=alpha;
  switch (imgcvt->png.colortype) {
    case 0: imgcvt->format=alpha?FMN_IMGFMT_ya11:FMN_IMGFMT_thumby; break;
    case 2: imgcvt->format=FMN_IMGFMT_rgba8888; break;
    case 3: imgcvt->format=FMN_IMGFMT_rgba8888; break;
    case 4: imgcvt->format=FMN_IMGFMT_ya11; break;
    case 6: imgcvt->format=FMN_IMGFMT_rgba8888; break;
    default: return -1;
  }
  return 0;
}

/* Receive one RGBA pixel from the PNG image.
 */
 
static struct imgcvt *gimgcvt;
 
static int imgcvt_cb_thumby_rgba(uint8_t r,uint8_t g,uint8_t b,uint8_t a,void *userdata) {
  struct fmn_image_iterator *iter=userdata;
  fmn_image_iterator_write(iter,((r+g+b)>=384)?1:0);
  if (!fmn_image_iterator_next(iter)) return 1;
  return 0;
}
 
static int imgcvt_cb_ya11_rgba(uint8_t r,uint8_t g,uint8_t b,uint8_t a,void *userdata) {
  struct fmn_image_iterator *iter=userdata;
  // When converting to ya11, intermediate grays count as transparent.
  uint8_t pixel=(r+g+b)/3;
  if (a<0x80) { pixel=0; gimgcvt->image.alpha=1; }
  else if ((pixel>=0x40)&&(pixel<0xc0)) { pixel=0; gimgcvt->image.alpha=1; }
  else if (pixel<0x80) pixel=1;
  else pixel=3;
  fmn_image_iterator_write(iter,pixel);
  if (!fmn_image_iterator_next(iter)) return 1;
  return 0;
}
 
static int imgcvt_cb_bgr565be_rgba(uint8_t r,uint8_t g,uint8_t b,uint8_t a,void *userdata) {
  struct fmn_image_iterator *iter=userdata;
  uint16_t pixel=((b<<8)&0xf800)|((g<<3)&0x07e0)|(r>>5);
  // Produce natural zeroes only if the pixel is actually transparent.
  if (a<0x80) { pixel=0; gimgcvt->image.alpha=1; }
  else if (!pixel) pixel=0x0800;
  fmn_image_iterator_write(iter,pixel);
  if (!fmn_image_iterator_next(iter)) return 1;
  return 0;
}
 
static int imgcvt_cb_rgba8888_rgba(uint8_t r,uint8_t g,uint8_t b,uint8_t a,void *userdata) {
  struct fmn_image_iterator *iter=userdata;
  if (!a) gimgcvt->image.alpha=1;
  fmn_image_iterator_write(iter,(r<<24)|(g<<16)|(b<<8)|a);
  if (!fmn_image_iterator_next(iter)) return 1;
  return 0;
}
 
static int imgcvt_cb_y1_rgba(uint8_t r,uint8_t g,uint8_t b,uint8_t a,void *userdata) {
  struct fmn_image_iterator *iter=userdata;
  fmn_image_iterator_write(iter,((r+g+b)>=384)?1:0);
  if (!fmn_image_iterator_next(iter)) return 1;
  return 0;
}
 
static int imgcvt_cb_y8_rgba(uint8_t r,uint8_t g,uint8_t b,uint8_t a,void *userdata) {
  struct fmn_image_iterator *iter=userdata;
  fmn_image_iterator_write(iter,(r+g+b)/3);
  if (!fmn_image_iterator_next(iter)) return 1;
  return 0;
}
 
static int imgcvt_cb_bgr332_rgba(uint8_t r,uint8_t g,uint8_t b,uint8_t a,void *userdata) {
  struct fmn_image_iterator *iter=userdata;
  uint8_t pixel=(b&0xe0)|((g>>3)&0x1c)|(r>>6);
  //TODO We have only so many colors, it would be great to be able to use natural black.
  // Find some other alpha strategy for bgr332.
  if (a<0x80) { gimgcvt->image.alpha=1; pixel=0; }
  else if (!pixel) pixel=0x20;
  fmn_image_iterator_write(iter,pixel);
  if (!fmn_image_iterator_next(iter)) return 1;
  return 0;
}

static int imgcvt_cb_argb4444be_rgba(uint8_t r,uint8_t g,uint8_t b,uint8_t a,void *userdata) {
  struct fmn_image_iterator *iter=userdata;
  if (a<0x80) gimgcvt->image.alpha=1;
  fmn_image_iterator_write(iter,((a<<8)&0xf000)|((r<<4)&0x0f00)|(g&0x00f0)|(b>>4));
  if (!fmn_image_iterator_next(iter)) return 1;
  return 0;
}

/* Fullmoon image from PNG, main entry point.
 */
 
int imgcvt_fmn_from_png(struct imgcvt *imgcvt) {
  gimgcvt=imgcvt;
  
  int err;
  if (!imgcvt->format) {
    if ((err=imgcvt_guess_format(imgcvt))<0) return err;
  }
  
  imgcvt->image.w=imgcvt->png.w;
  imgcvt->image.h=imgcvt->png.h;
  imgcvt->image.fmt=imgcvt->format;
  imgcvt->image.writeable=1;
  int (*rcvpx)(uint8_t,uint8_t,uint8_t,uint8_t,void*)=0;
  switch (imgcvt->format) {
    case FMN_IMGFMT_thumby: {
        imgcvt->image.stride=imgcvt->image.w;
        imgcvt->image.v=calloc(imgcvt->image.w,((imgcvt->image.h+7)>>3));
        rcvpx=imgcvt_cb_thumby_rgba;
      } break;
    case FMN_IMGFMT_ya11: {
        imgcvt->image.stride=(imgcvt->image.w+3)>>2;
        imgcvt->image.v=calloc(imgcvt->image.stride,imgcvt->image.h);
        rcvpx=imgcvt_cb_ya11_rgba;
      } break;
    case FMN_IMGFMT_bgr565be: {
        imgcvt->image.stride=imgcvt->image.w*2;
        imgcvt->image.v=calloc(imgcvt->image.stride,imgcvt->image.h);
        rcvpx=imgcvt_cb_bgr565be_rgba;
      } break;
    case FMN_IMGFMT_rgba8888: {
        imgcvt->image.stride=imgcvt->image.w*4;
        imgcvt->image.v=calloc(imgcvt->image.stride,imgcvt->image.h);
        rcvpx=imgcvt_cb_rgba8888_rgba;
      } break;
    case FMN_IMGFMT_y1: {
        imgcvt->image.stride=(imgcvt->image.w+7)>>3;
        imgcvt->image.v=calloc(imgcvt->image.stride,imgcvt->image.h);
        rcvpx=imgcvt_cb_y1_rgba;
      } break;
    case FMN_IMGFMT_y8: {
        imgcvt->image.stride=imgcvt->image.w;
        imgcvt->image.v=calloc(imgcvt->image.stride,imgcvt->image.h);
        rcvpx=imgcvt_cb_y8_rgba;
      } break;
    case FMN_IMGFMT_bgr332: {
        imgcvt->image.stride=imgcvt->image.w;
        imgcvt->image.v=calloc(imgcvt->image.stride,imgcvt->image.h);
        rcvpx=imgcvt_cb_bgr332_rgba;
      } break;
    case FMN_IMGFMT_argb4444be: {
        imgcvt->image.stride=imgcvt->image.w*2;
        imgcvt->image.v=calloc(imgcvt->image.stride,imgcvt->image.h);
        rcvpx=imgcvt_cb_argb4444be_rgba;
      } break;
    default: {
        fprintf(stderr,"%s:%d: Unsupported image format %d\n",__FILE__,__LINE__,imgcvt->format);
        return -2;
      }
  }
  if (!imgcvt->image.v) return -1;
  
  struct fmn_image_iterator iter={0};
  if (!fmn_image_iterate(&iter,&imgcvt->image,0,0,imgcvt->image.w,imgcvt->image.h,0)) return -1;
  if (png_image_iterate_rgba8888(&imgcvt->png,rcvpx,&iter)<0) return -1;
  imgcvt->image.writeable=0;
  
  return 0;
}
