#include "png.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <zlib.h>

/* Private decoder.
 */
 
struct png_decoder {
  struct png_image *image; // WEAK
  z_stream z;
  int zinit;
  int y;
  uint8_t *rowbuf;
  int xstride;
};

/* Cleanup.
 */

void png_image_cleanup(struct png_image *image) {
  if (image->pixels) free(image->pixels);
  if (image->plte) free(image->plte);
  if (image->trns) free(image->trns);
}

static void png_decoder_cleanup(struct png_decoder *decoder) {
  if (decoder->zinit) inflateEnd(&decoder->z);
  if (decoder->rowbuf) free(decoder->rowbuf);
}

/* Receive one filtered row, uncompressed.
 * Read from (decoder->rowbuf) and advance (decoder->y).
 */
 
static inline void png_unfilter_NONE(uint8_t *dst,const uint8_t *src,const uint8_t *prv,int stride,int xstride) {
  memcpy(dst,src,stride);
}
 
static inline void png_unfilter_SUB(uint8_t *dst,const uint8_t *src,const uint8_t *prv,int stride,int xstride) {
  int i=0;
  for (;i<xstride;i++) dst[i]=src[i];
  for (;i<stride;i++) dst[i]=src[i]+dst[i-xstride];
}
 
static inline void png_unfilter_UP(uint8_t *dst,const uint8_t *src,const uint8_t *prv,int stride,int xstride) {
  if (!prv) memcpy(dst,src,stride);
  else {
    int i=0;
    for (;i<stride;i++) dst[i]=src[i]+prv[i];
  }
}
 
static inline void png_unfilter_AVG(uint8_t *dst,const uint8_t *src,const uint8_t *prv,int stride,int xstride) {
  int i=0;
  if (!prv) {
    for (;i<xstride;i++) dst[i]=src[i];
    for (;i<stride;i++) dst[i]=src[i]+(dst[i-xstride]>>1);
  } else {
    for (;i<xstride;i++) dst[i]=src[i]+(prv[i]>>1);
    for (;i<stride;i++) dst[i]=src[i]+((prv[i]+dst[i-xstride])>>1);
  }
}

static inline uint8_t png_paeth(uint8_t a,uint8_t b,uint8_t c) {
  int p=a+b-c;
  int pa=p-a; if (pa<0) pa=-pa;
  int pb=p-b; if (pb<0) pb=-pb;
  int pc=p-c; if (pc<0) pc=-pc;
  if ((pa<=pb)&&(pa<=pc)) return a;
  if (pb<=pc) return b;
  return c;
}
 
static inline void png_unfilter_PAETH(uint8_t *dst,const uint8_t *src,const uint8_t *prv,int stride,int xstride) {
  int i=0;
  if (!prv) {
    for (;i<xstride;i++) dst[i]=src[i];
    for (;i<stride;i++) dst[i]=src[i]+dst[i-xstride];
  } else {
    for (;i<xstride;i++) dst[i]=src[i]+prv[i];
    for (;i<stride;i++) dst[i]=src[i]+png_paeth(dst[i-xstride],prv[i],prv[i-xstride]);
  }
}
 
static int png_decode_row(struct png_decoder *decoder) {
  if (decoder->y>=decoder->image->h) return 0; // discard extra data
  uint8_t *dst=decoder->image->pixels+decoder->y*decoder->image->stride;
  uint8_t *prv=0;
  if (decoder->y>0) prv=dst-decoder->image->stride;
  const uint8_t *src=decoder->rowbuf+1;
  switch (decoder->rowbuf[0]) {
    case 0: png_unfilter_NONE(dst,src,prv,decoder->image->stride,decoder->xstride); break;
    case 1: png_unfilter_SUB(dst,src,prv,decoder->image->stride,decoder->xstride); break;
    case 2: png_unfilter_UP(dst,src,prv,decoder->image->stride,decoder->xstride); break;
    case 3: png_unfilter_AVG(dst,src,prv,decoder->image->stride,decoder->xstride); break;
    case 4: png_unfilter_PAETH(dst,src,prv,decoder->image->stride,decoder->xstride); break;
    default: return -1;
  }
  decoder->y++;
  return 0;
}

/* Receive IDAT.
 */
 
static int png_decode_IDAT(struct png_decoder *decoder,const void *src,int srcc) {
  if (!decoder->image->pixels) return -1;
  decoder->z.next_in=(Bytef*)src;
  decoder->z.avail_in=srcc;
  while (decoder->z.avail_in>0) {
  
    if (!decoder->z.avail_out) {
      if (png_decode_row(decoder)<0) return -1;
      decoder->z.next_out=(Bytef*)decoder->rowbuf;
      decoder->z.avail_out=1+decoder->image->stride;
    }
    
    int err=inflate(&decoder->z,Z_NO_FLUSH);
    if (err<0) return -1;
  }
  return 0;
}

/* Receive IHDR.
 */
 
static int png_decode_IHDR(struct png_decoder *decoder,const uint8_t *src,int srcc) {
  if (decoder->image->pixels) return -1;
  if (srcc<13) return -1;
  
  int w=(src[0]<<24)|(src[1]<<16)|(src[2]<<8)|src[3];
  int h=(src[4]<<24)|(src[5]<<16)|(src[6]<<8)|src[7];
  if ((w<1)||(w>PNG_SIZE_LIMIT)) return -1;
  if ((h<1)||(h>PNG_SIZE_LIMIT)) return -1;
  
  if (src[10]||src[11]||src[12]) {
    // (filter,compression,interlace)
    // (filter,compression) can only be zero.
    // (interlace) could also be 1, Adam7, but we're not doing that.
    return -1;
  }
  
  decoder->image->depth=src[8];
  decoder->image->colortype=src[9];
  
  // This formula permits a few illegal things eg rgba4444, but we don't mind.
  int chanc;
  switch (decoder->image->colortype) {
    case 0: chanc=1; break;
    case 2: chanc=3; break;
    case 3: chanc=1; break;
    case 4: chanc=2; break;
    case 6: chanc=4; break;
    default: return -1;
  }
  int pixelsize=decoder->image->depth*chanc;
  switch (pixelsize) {
    case 1: case 2: case 4: case 8:
    case 16: case 24: case 32: case 48: case 64:
      break;
    default: return -1;
  }
  
  decoder->image->stride=(pixelsize*w+7)>>3;
  if (!(decoder->image->pixels=malloc(decoder->image->stride*h))) return -1;
  decoder->image->w=w;
  decoder->image->h=h;
  
  decoder->xstride=(pixelsize+7)>>3;
  if (!(decoder->rowbuf=malloc(1+decoder->image->stride))) return -1;
  
  if (inflateInit(&decoder->z)<0) return -1;
  decoder->zinit=1;
  decoder->z.next_out=(Bytef*)decoder->rowbuf;
  decoder->z.avail_out=1+decoder->image->stride;
  
  return 0;
}

/* Receive verbatim chunks PLTE,tRNS
 */
 
static int png_decode_PLTE(struct png_decoder *decoder,const void *src,int srcc) {
  if (decoder->image->plte) return -1;
  if (!srcc) return 0;
  if (!(decoder->image->plte=malloc(srcc))) return -1;
  memcpy(decoder->image->plte,src,srcc);
  decoder->image->pltec=srcc;
  return 0;
}

static int png_decode_tRNS(struct png_decoder *decoder,const void *src,int srcc) {
  if (decoder->image->trns) return -1;
  if (!srcc) return 0;
  if (!(decoder->image->trns=malloc(srcc))) return -1;
  memcpy(decoder->image->trns,src,srcc);
  decoder->image->trnsc=srcc;
  return 0;
}

/* Finish decode.
 */
 
static int png_decode_finish(struct png_decoder *decoder) {
  if (!decoder->image->pixels) return -1;
  
  decoder->z.avail_in=0;
  while (decoder->y<decoder->image->h) {
    if (!decoder->z.avail_out) {
      decoder->z.next_out=(Bytef*)decoder->rowbuf;
      decoder->z.avail_out=1+decoder->image->stride;
    }
    int err=inflate(&decoder->z,Z_FINISH);
    if (err<0) return -1;
    if (png_decode_row(decoder)<0) return -1;
  }
  
  return 0;
}

/* Decode, public entry point.
 */

int png_image_decode(struct png_image *image,const void *src,int srcc) {
  if (!image||image->pixels) return -1; // (image->pixels) is a proxy for "IHDR present"
  if (!src||(srcc<8)||memcmp(src,"\x89PNG\r\n\x1a\n",8)) return -1;
  struct png_decoder decoder={
    .image=image,
  };
  const uint8_t *SRC=src;
  int srcp=8;
  while (srcp<srcc) {
    if (srcp>srcc-8) return -1;
    int len=(SRC[srcp]<<24)|(SRC[srcp+1]<<16)|(SRC[srcp+2]<<8)|SRC[srcp+3];
    const uint8_t *chunkid=SRC+srcp+4;
    srcp+=8;
    if ((len<0)||(srcp>srcc-len)) return -1;
    int err=0;
         if (!memcmp(chunkid,"IHDR",4)) err=png_decode_IHDR(&decoder,SRC+srcp,len);
    else if (!memcmp(chunkid,"IDAT",4)) err=png_decode_IDAT(&decoder,SRC+srcp,len);
    else if (!memcmp(chunkid,"PLTE",4)) err=png_decode_PLTE(&decoder,SRC+srcp,len);
    else if (!memcmp(chunkid,"tRNS",4)) err=png_decode_tRNS(&decoder,SRC+srcp,len);
    // Don't bother looking at IEND or unknown chunks.
    if (err<0) return -1;
    srcp+=len;
    srcp+=4; // skip CRC
  }
  int err=png_decode_finish(&decoder);
  png_decoder_cleanup(&decoder);
  return err;
}
