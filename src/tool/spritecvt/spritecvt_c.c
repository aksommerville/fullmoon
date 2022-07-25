#include "spritecvt.h"

/* Name of C object from path (or command line or something?).
 */
 
static int spritecvt_validate_c_object_name(const char *src,int srcc) {
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
 
static int spritecvt_get_c_object_name(char *dst,int dsta,struct spritecvt *spritecvt) {
  
  const char *src=spritecvt->srcpath;
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
    return spritecvt_validate_c_object_name(stem,stemc);
  }
  
  return -1;
}

/* Encode in context.
 */
 
static int spritecvt_encode_c_to_encoder(struct encoder *dst,struct spritecvt *spritecvt) {
  int err;
  
  char name[64];
  int namec=spritecvt_get_c_object_name(name,sizeof(name),spritecvt);
  if ((namec<1)||(namec>sizeof(name))) {
    fprintf(stderr,"%s: Failed to determine a valid C identifier for output object.\n",spritecvt->srcpath);
    return -2;
  }

  if (encode_fmt(dst,"#include <stdint.h>\n")<0) return -1;
  if (encode_fmt(dst,"#include \"game/fullmoon.h\"\n")<0) return -1;
  if (encode_fmt(dst,"#include \"game/sprite/fmn_sprite.h\"\n")<0) return -1;
  
  // No need to extern 'tname'; all sprtype names must be declared in fmn_sprite.h
  if (spritecvt->imagenamec) {
    if (encode_fmt(dst,"extern const struct fmn_image %.*s;\n",spritecvt->imagenamec,spritecvt->imagename)<0) return -1;
  }
  
  if (encode_fmt(dst,"const struct fmn_sprdef %.*s={\n",namec,name)<0) return -1;
  if (encode_fmt(dst,"  .type=&fmn_sprtype_%.*s,\n",spritecvt->tnamec,spritecvt->tname)<0) return -1;
  if (spritecvt->imagenamec) {
    if (encode_fmt(dst,"  .image=&%.*s,\n",spritecvt->imagenamec,spritecvt->imagename)<0) return -1;
  }
  if (encode_fmt(dst,"  .tileid=%d,\n",spritecvt->tileid)<0) return -1;
  if (encode_fmt(dst,"  .xform=%d,\n",spritecvt->xform)<0) return -1;
  if (encode_fmt(dst,"};\n")<0) return -1;
  
  return 0;
}

/* Encode map object to C text.
 */
 
int spritecvt_encode_c(struct spritecvt *spritecvt) {
  if (spritecvt->dst) return -1;
  struct encoder dst={0};
  int err=spritecvt_encode_c_to_encoder(&dst,spritecvt);
  spritecvt->dst=dst.v; // pass or fail; so it gets cleaned up
  spritecvt->dstc=dst.c;
  return err;
}
