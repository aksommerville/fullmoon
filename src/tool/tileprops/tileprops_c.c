#include "tileprops.h"

/* Name of C object from path (or command line or something?).
 */
 
static int tileprops_validate_c_object_name(const char *src,int srcc) {
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
 
static int tileprops_get_c_object_name(char *dst,int dsta,struct tileprops *tileprops) {
  
  const char *src=tileprops->srcpath;
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
    return tileprops_validate_c_object_name(stem,stemc);
  }
  
  return -1;
}

/* Encode in context.
 */
 
static int tileprops_encode_c_to_encoder(struct encoder *dst,struct tileprops *tileprops) {
  int err;

  if (encode_fmt(dst,"#include <stdint.h>\n")<0) return -1;
  
  char name[64];
  int namec=tileprops_get_c_object_name(name,sizeof(name),tileprops);
  if ((namec<1)||(namec>sizeof(name))) {
    fprintf(stderr,"%s: Failed to determine a valid C identifier for output object.\n",tileprops->srcpath);
    return -2;
  }
  
  // Only "props" gets encoded. The other three blocks are for editor only.
  const int LINE_LENGTH_LIMIT=100;
  if (encode_fmt(dst,"const uint8_t %.*s[]={\n",namec,name)<0) return -1;
  int linelen=0;
  const uint8_t *v=tileprops->props;
  int i=256;
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
  
  return 0;
}

/* Encode map object to C text.
 */
 
int tileprops_encode_c(struct tileprops *tileprops) {
  if (tileprops->dst) return -1;
  struct encoder dst={0};
  int err=tileprops_encode_c_to_encoder(&dst,tileprops);
  tileprops->dst=dst.v; // pass or fail; so it gets cleaned up
  tileprops->dstc=dst.c;
  return err;
}
