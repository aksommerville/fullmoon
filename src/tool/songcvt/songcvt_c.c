#include "songcvt.h"

/* Name of C object from path (or command line or something?).
 */
 
static int songcvt_validate_c_object_name(const char *src,int srcc) {
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
 
static int songcvt_get_c_object_name(char *dst,int dsta,struct songcvt *songcvt) {
  
  const char *src=songcvt->srcpath;
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
    return songcvt_validate_c_object_name(stem,stemc);
  }
  
  return -1;
}

/* Encode in context.
 */
 
static int songcvt_encode_c_to_encoder(struct encoder *dst,struct songcvt *songcvt) {
  int err;

  if (encode_fmt(dst,"#include <stdint.h>\n")<0) return -1;
  
  char name[64];
  int namec=songcvt_get_c_object_name(name,sizeof(name),songcvt);
  if ((namec<1)||(namec>sizeof(name))) {
    fprintf(stderr,"%s: Failed to determine a valid C identifier for output object.\n",songcvt->srcpath);
    return -2;
  }
  
  const int LINE_LENGTH_LIMIT=100;
  if (encode_fmt(dst,"const uint8_t song_%.*s[]%s={\n",namec,name,songcvt->progmem?" PROGMEM":"")<0) return -1;
  int linelen=0;
  const uint8_t *v=songcvt->bin.v;
  int i=songcvt->bin.c;
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
  
  if (encode_fmt(dst,"\nconst uint16_t song_%.*s_length=%d;\n",namec,name,songcvt->bin.c)<0) return -1;
  
  return 0;
}

/* Encode map object to C text.
 */
 
int songcvt_encode_c(struct songcvt *songcvt) {
  if (songcvt->dst) return -1;
  
  if (songcvt->bin.c>0xffff) {
    fprintf(stderr,"%s: Output song exceeds 64 kB (%d)\n",songcvt->srcpath,songcvt->bin.c);
    return -2;
  }
  
  struct encoder dst={0};
  int err=songcvt_encode_c_to_encoder(&dst,songcvt);
  songcvt->dst=dst.v; // pass or fail; so it gets cleaned up
  songcvt->dstc=dst.c;
  return err;
}
