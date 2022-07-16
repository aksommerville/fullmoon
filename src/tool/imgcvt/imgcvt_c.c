#include "imgcvt.h"

/* Measure image.
 */
 
int fmn_image_get_size(const struct fmn_image *image) {
  switch (image->fmt) {
    case FMN_IMGFMT_thumby: return image->stride*((image->h+7)>>3);
    default: return image->stride*image->h;
  }
}

/* Name of C object from path (or command line or something?).
 */
 
static int imgcvt_validate_c_object_name(const char *src,int srcc) {
  if (srcc<1) return -1;
  if ((src[0]>='0')&&(src[0]<='9')) return -1;
  int i=srcc; for (;i-->0;src++) {
    if ((*src>='a')&&(*src<='z')) continue;
    if ((*src>='A')&&(*src<='Z')) continue;
    if ((*src>='0')&&(*src<='9')) continue;
    if (*src=='_') continue;
    return -1;
  }
  return srcc;
}
 
static int imgcvt_get_c_object_name(char *dst,int dsta,struct imgcvt *imgcvt) {
  
  const char *src=imgcvt->srcpath;
  if (src) {
    const char *stem=src;
    int stemc=0,cp=1,srcp=0;
    for (;src[srcp];srcp++) {
      if ((src[srcp]=='/')||(src[srcp]=='\\')) {
        stem=src+srcp+1;
        stemc=0;
        cp=1;
      } else if ((src[srcp]=='-')||(src[srcp]=='.')) {
        cp=0;
      } else if (cp) {
        stemc++;
      }
    }
    if (stemc<=dsta) memcpy(dst,stem,stemc);
    if (stemc<dsta) dst[stemc]=0;
    return imgcvt_validate_c_object_name(stem,stemc);
  }
  
  return -1;
}

/* Encode in context.
 */
 
static int imgcvt_encode_c_to_encoder(struct encoder *dst,struct imgcvt *imgcvt) {

  if (encode_fmt(dst,"#include <stdint.h>\n")<0) return -1;
  if (encode_fmt(dst,"#include \"game/fullmoon.h\"\n")<0) return -1;
  
  char name[64];
  int namec=imgcvt_get_c_object_name(name,sizeof(name),imgcvt);
  if ((namec<1)||(namec>sizeof(name))) {
    fprintf(stderr,"%s: Failed to determine a valid C identifier for output object.\n",imgcvt->srcpath);
    return -2;
  }
  
  const int LINE_LENGTH_LIMIT=100;
  if (encode_fmt(dst,"static const uint8_t %.*s_STORAGE[]={\n",namec,name)<0) return -1;
  int linelen=0;
  const uint8_t *v=imgcvt->image.v;
  int i=fmn_image_get_size(&imgcvt->image);
  for (;i-->0;v++) {
    int err=encode_fmt(dst,"%d,",*v);
    if (err<0) return -1;
    linelen+=err;
    if (linelen>=LINE_LENGTH_LIMIT) {
      encode_raw(dst,"\n",1);
      linelen=0;
    }
  }
  if (linelen) encode_raw(dst,"\n",1);
  if (encode_fmt(dst,"};\n")<0) return -1;
  
  if (encode_fmt(dst,"const struct fmn_image %.*s={\n",namec,name)<0) return -1;
  if (encode_fmt(dst,"  .v=(void*)%.*s_STORAGE,\n",namec,name)<0) return -1;
  if (encode_fmt(dst,"  .w=%d,\n",imgcvt->image.w)<0) return -1;
  if (encode_fmt(dst,"  .h=%d,\n",imgcvt->image.h)<0) return -1;
  if (encode_fmt(dst,"  .fmt=%d,\n",imgcvt->image.fmt)<0) return -1;
  if (encode_fmt(dst,"  .stride=%d,\n",imgcvt->image.stride)<0) return -1;
  if (encode_fmt(dst,"  .writeable=0,\n")<0) return -1;
  if (encode_fmt(dst,"};\n")<0) return -1;
  
  return 0;
}

/* Encode fmn image as a C array and struct.
 */
 
int imgcvt_encode_c(struct imgcvt *imgcvt) {
  if (imgcvt->dst) return -1;
  struct encoder dst={0};
  int err=imgcvt_encode_c_to_encoder(&dst,imgcvt);
  imgcvt->dst=dst.v; // pass or fail; so it gets cleaned up
  imgcvt->dstc=dst.c;
  return err;
}
