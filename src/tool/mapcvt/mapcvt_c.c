#include "mapcvt.h"
#include "mapcvt.h"

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
 
static int mapcvt_validate_c_object_name(const char *src,int srcc) {
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
 
static int mapcvt_get_c_object_name(char *dst,int dsta,struct mapcvt *mapcvt) {
  
  const char *src=mapcvt->srcpath;
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
    return mapcvt_validate_c_object_name(stem,stemc);
  }
  
  return -1;
}

/* Encode in context.
 */
 
static int mapcvt_encode_c_to_encoder(struct encoder *dst,struct mapcvt *mapcvt) {

  if (encode_fmt(dst,"#include <stdint.h>\n")<0) return -1;
  if (encode_fmt(dst,"#include \"game/fullmoon.h\"\n")<0) return -1;
  
  char name[64];
  int namec=mapcvt_get_c_object_name(name,sizeof(name),mapcvt);
  if ((namec<1)||(namec>sizeof(name))) {
    fprintf(stderr,"%s: Failed to determine a valid C identifier for output object.\n",mapcvt->srcpath);
    return -2;
  }
  
  const int LINE_LENGTH_LIMIT=100;
  if (encode_fmt(dst,"static const uint8_t %.*s_STORAGE[]={\n",namec,name)<0) return -1;
  int linelen=0;
  const uint8_t *v=mapcvt->map.v;
  int i=mapcvt->map.w*mapcvt->map.h;
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
  
  if (encode_fmt(dst,"const struct fmn_map %.*s={\n",namec,name)<0) return -1;
  if (encode_fmt(dst,"  .v=(void*)%.*s_STORAGE,\n",namec,name)<0) return -1;
  if (encode_fmt(dst,"  .w=%d,\n",mapcvt->map.w)<0) return -1;
  if (encode_fmt(dst,"  .h=%d,\n",mapcvt->map.h)<0) return -1;
  if (encode_fmt(dst,"  .initx=%d,\n",mapcvt->map.initx)<0) return -1;
  if (encode_fmt(dst,"  .inity=%d,\n",mapcvt->map.inity)<0) return -1;
  if (encode_fmt(dst,"};\n")<0) return -1;
  
  return 0;
}

/* Encode map object to C text.
 */
 
int mapcvt_encode_c(struct mapcvt *mapcvt) {
  if (mapcvt->dst) return -1;
  struct encoder dst={0};
  int err=mapcvt_encode_c_to_encoder(&dst,mapcvt);
  mapcvt->dst=dst.v; // pass or fail; so it gets cleaned up
  mapcvt->dstc=dst.c;
  return err;
}
