#include "png.h"

/* Iterate, converting to ya88.
 */
 
int png_image_iterate_ya88(
  const struct png_image *image,
  int (*cb)(uint8_t y,uint8_t a,void *userdata),
  void *userdata
) {
  int err;
  const uint8_t *row=image->pixels;
  int yi=image->h;
  #define ITERATE(eachrow,eachcol) \
    for (;yi-->0;row+=image->stride) { \
      const uint8_t *p=row; \
      int xi=image->w; \
      uint8_t y=0,a=0; \
      uint8_t mask,shift; \
      eachrow \
      for (;xi-->0;) { \
        eachcol \
        if (err=cb(y,a,userdata)) return err; \
      } \
    } \
    return 0;
  switch (image->colortype) {
    case 0: _gray_: switch (image->depth) {
        case 1: ITERATE({
            mask=0x80;
            a=0xff;
          },{
            if ((*p)&mask) y=0xff;
            else y=0x00;
            if (!(mask>>=1)) {
              mask=0x80;
              p++;
            }
          })
        case 2: ITERATE({
            shift=6;
            a=0xff;
          },{
            y=((*p)>>shift)&3;
            y|=y<<2;
            y|=y<<4;
            if (shift) shift-=2;
            else { shift=6; p++; }
          })
        case 4: ITERATE({
            shift=4;
            a=0xff;
          },{
            y=((*p)>>shift)&15;
            y|=y<<4;
            if (shift) shift=0;
            else { shift=4; p++; }
          })
        case 8: ITERATE({ a=0xff; },{ y=*p; p+=1; })
        case 16: ITERATE({ a=0xff; },{ y=*p; p+=2; })
      } break;
    case 2: switch (image->depth) {
        case 8: ITERATE({ a=0xff; },{ y=(p[0]+p[1]+p[2])/3; p+=3; })
        case 16: ITERATE({ a=0xff; },{ y=(p[0]+p[2]+p[4])/3; p+=6; })
      } break;
    case 3: if (!image->pltec) goto _gray_; switch (image->depth) {
        case 1: ITERATE({
            mask=0x80;
          },{
            if ((*p)&mask) y=3;
            else y=0;
            if (y+3<=image->pltec) y=(image->plte[y*3]+image->plte[y*3+1]+image->plte[y*3+2])/3;
            else y=0;
            if (y<image->trnsc) a=image->trns[y];
            else a=0xff;
            if (!(mask>>=1)) {
              mask=0x80;
              p++;
            }
          })
        case 2: ITERATE({
            shift=6;
          },{
            y=((*p)>>shift)&3;
            if (y+3<=image->pltec) y=(image->plte[y*3]+image->plte[y*3+1]+image->plte[y*3+2])/3;
            else y=0;
            if (y<image->trnsc) a=image->trns[y];
            else a=0xff;
            if (shift) shift-=2;
            else { shift=6; p++; }
          })
        case 4: ITERATE({
            shift=4;
          },{
            y=((*p)>>shift)&15;
            if (y+3<=image->pltec) y=(image->plte[y*3]+image->plte[y*3+1]+image->plte[y*3+2])/3;
            else y=0;
            if (y<image->trnsc) a=image->trns[y];
            else a=0xff;
            if (shift) shift=0;
            else { shift=4; p++; }
          })
        case 8: ITERATE({},{
            y=*p;
            if (y+3<=image->pltec) y=(image->plte[y*3]+image->plte[y*3+1]+image->plte[y*3+2])/3;
            else y=0;
            if (y<image->trnsc) a=image->trns[y];
            else a=0xff;
            p++;
          })
      } break;
    case 4: switch (image->depth) {
        case 8: ITERATE({},{ y=p[0]; a=p[1]; p+=2; })
        case 16: ITERATE({},{ y=p[0]; a=p[2]; p+=4; })
      } break;
    case 6: switch (image->depth) {
        case 8: ITERATE({},{ y=(p[0]+p[1]+p[2])/3; a=p[3]; p+=4; })
        case 16: ITERATE({},{ y=(p[0]+p[2]+p[4])/3; a=p[6]; p+=8; })
      } break;
  }
  #undef ITERATE
  return -1;
}

/* Iterate, converting to rgba8888.
 */
 
int png_image_iterate_rgba8888(
  const struct png_image *image,
  int (*cb)(uint8_t r,uint8_t g,uint8_t b,uint8_t a,void *userdata),
  void *userdata
) {
  int err;
  const uint8_t *row=image->pixels;
  int yi=image->h;
  #define ITERATE(eachrow,eachcol) \
    for (;yi-->0;row+=image->stride) { \
      const uint8_t *p=row; \
      int xi=image->w; \
      uint8_t r=0,g=0,b=0,a=0; \
      uint8_t mask,shift; \
      eachrow \
      for (;xi-->0;) { \
        eachcol \
        if (err=cb(r,g,b,a,userdata)) return err; \
      } \
    } \
    return 0;
  switch (image->colortype) {
    case 0: _gray_: switch (image->depth) {
        case 1: ITERATE({
            mask=0x80;
            a=0xff;
          },{
            if ((*p)&mask) r=g=b=0xff;
            else r=g=b=0x00;
            if (!(mask>>=1)) {
              mask=0x80;
              p++;
            }
          })
        case 2: ITERATE({
            shift=6;
            a=0xff;
          },{
            r=((*p)>>shift)&3;
            r|=r<<2;
            r|=r<<4;
            g=b=r;
            if (shift) shift-=2;
            else { shift=6; p++; }
          })
        case 4: ITERATE({
            shift=4;
            a=0xff;
          },{
            r=((*p)>>shift)&15;
            r|=r<<4;
            g=b=r;
            if (shift) shift=0;
            else { shift=4; p++; }
          })
        case 8: ITERATE({ a=0xff; },{ r=g=b=*p; p+=1; })
        case 16: ITERATE({ a=0xff; },{ r=g=b=*p; p+=2; })
      } break;
    case 2: switch (image->depth) {
        case 8: ITERATE({ a=0xff; },{ r=p[0]; g=p[1]; b=p[2]; p+=3; })
        case 16: ITERATE({ a=0xff; },{ r=p[0]; g=p[2]; b=p[4]; p+=6; })
      } break;
    case 3: if (!image->pltec) goto _gray_; switch (image->depth) {
        case 1: ITERATE({
            mask=0x80;
          },{
            uint8_t ix;
            if ((*p)&mask) ix=3;
            else ix=0;
            if (ix+3<=image->pltec) { r=image->plte[ix]; g=image->plte[ix+1]; b=image->plte[ix+2]; }
            else r=g=b=0;
            if (ix<image->trnsc) a=image->trns[ix];
            else a=0xff;
            if (!(mask>>=1)) {
              mask=0x80;
              p++;
            }
          })
        case 2: ITERATE({
            shift=6;
          },{
            uint8_t ix=((*p)>>shift)&3;
            if (ix+3<=image->pltec) { r=image->plte[ix]; g=image->plte[ix+1]; b=image->plte[ix+2]; }
            else r=g=b=0;
            if (ix<image->trnsc) a=image->trns[ix];
            else a=0xff;
            if (shift) shift-=2;
            else { shift=6; p++; }
          })
        case 4: ITERATE({
            shift=4;
          },{
            uint8_t ix=((*p)>>shift)&15;
            if (ix+3<=image->pltec) { r=image->plte[ix]; g=image->plte[ix+1]; b=image->plte[ix+2]; }
            else r=g=b=0;
            if (ix<image->trnsc) a=image->trns[ix];
            else a=0xff;
            if (shift) shift=0;
            else { shift=4; p++; }
          })
        case 8: ITERATE({},{
            uint8_t ix=*p;
            if (ix+3<=image->pltec) { r=image->plte[ix]; g=image->plte[ix+1]; b=image->plte[ix+2]; }
            else r=g=b=0;
            if (ix<image->trnsc) a=image->trns[ix];
            else a=0xff;
            p++;
          })
      } break;
    case 4: switch (image->depth) {
        case 8: ITERATE({},{ r=g=b=p[0]; a=p[1]; p+=2; })
        case 16: ITERATE({},{ r=g=b=p[0]; a=p[2]; p+=4; })
      } break;
    case 6: switch (image->depth) {
        case 8: ITERATE({},{ r=p[0]; g=p[1]; b=p[2]; a=p[3]; p+=4; })
        case 16: ITERATE({},{ r=p[0]; g=p[2]; b=p[4]; a=p[6]; p+=8; })
      } break;
  }
  #undef ITERATE
  return -1;
}
