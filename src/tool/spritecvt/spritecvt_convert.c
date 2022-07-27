#include "spritecvt.h"
#include "game/sprite/fmn_sprite.h"
#include "tool/common/serial.h"

/* Generic integer field.
 */
 
static int spritecvt_parse_int(
  int *dst,struct spritecvt *spritecvt,const char *src,int srcc,
  int lineno,const char *k,int lo,int hi
) {
  if (int_eval(dst,src,srcc)<2) {
    fprintf(stderr,"%s:%d: Failed to evaluate '%.*s' as integer for '%s'\n",spritecvt->srcpath,lineno,srcc,src,k);
    return -2;
  }
  if ((*dst<lo)||(*dst>hi)) {
    fprintf(stderr,"%s:%d: %d out of range for '%s', limit %d..%d\n",spritecvt->srcpath,lineno,*dst,k,lo,hi);
    return -2;
  }
  return 0;
}

/* String fields.
 */
 
static int spritecvt_parse_type(struct spritecvt *spritecvt,const char *src,int srcc,int lineno) {
  if (spritecvt->tname) {
    fprintf(stderr,"%s:%d: duplicate type\n",spritecvt->srcpath,lineno);
    return -2;
  }
  if (!(spritecvt->tname=malloc(srcc+1))) return -1;
  memcpy(spritecvt->tname,src,srcc);
  spritecvt->tname[srcc]=0;
  spritecvt->tnamec=srcc;
  return 0;
}
 
static int spritecvt_parse_image(struct spritecvt *spritecvt,const char *src,int srcc,int lineno) {
  if (spritecvt->imagename) {
    fprintf(stderr,"%s:%d: duplicate image\n",spritecvt->srcpath,lineno);
    return -2;
  }
  if (!(spritecvt->imagename=malloc(srcc+1))) return -1;
  memcpy(spritecvt->imagename,src,srcc);
  spritecvt->imagename[srcc]=0;
  spritecvt->imagenamec=srcc;
  return 0;
}

/* xform: Permit a few strings that only exist here.
 */
 
static int spritecvt_parse_xform(struct spritecvt *spritecvt,const char *src,int srcc,int lineno) {
  // Plain integers 0..7 are just fine.
  if (int_eval(&spritecvt->xform,src,srcc)>=2) {
    if ((spritecvt->xform<0)||(spritecvt->xform>7)) {
      fprintf(stderr,"%s:%d: xform must be in 0..7, found %d\n",spritecvt->srcpath,lineno,spritecvt->xform);
      return -2;
    }
    return 0;
  }
  // Also accept space-delimited xform flag names.
  // I'll stop short of aggregate constructions like "rotate90". User must speak our language.
  spritecvt->xform=0;
  int srcp=0;
  while (srcp<srcc) {
    if ((unsigned char)src[srcp]<=0x20) { srcp++; continue; }
    const char *token=src+srcp;
    int tokenc=0;
    while ((srcp<srcc)&&((unsigned char)src[srcp]>0x20)) { srcp++; tokenc++; }
    
         if ((tokenc==4)&&!memcmp(token,"XREV",4)) spritecvt->xform|=FMN_XFORM_XREV;
    else if ((tokenc==4)&&!memcmp(token,"YREV",4)) spritecvt->xform|=FMN_XFORM_YREV;
    else if ((tokenc==4)&&!memcmp(token,"SWAP",4)) spritecvt->xform|=FMN_XFORM_SWAP;
    
    else {
      fprintf(stderr,
        "%s:%d: Unexpected xform '%.*s'. Please provide 0..7, or a combination of 'XREV','YREV','SWAP'\n",
        spritecvt->srcpath,lineno,tokenc,token
      );
      return -2;
    }
  }
  return 0;
}

/* flags: Permit a few strings that only exist here.
 */
 
static int spritecvt_parse_flags(struct spritecvt *spritecvt,const char *src,int srcc,int lineno) {
  // Plain integers 0..0xffff are just fine.
  if (int_eval(&spritecvt->flags,src,srcc)>=2) {
    if ((spritecvt->flags<0)||(spritecvt->flags>0xffff)) {
      fprintf(stderr,"%s:%d: xform must be in 0..65535, found %d\n",spritecvt->srcpath,lineno,spritecvt->flags);
      return -2;
    }
    return 0;
  }
  // Also accept space-delimited flag names.
  spritecvt->flags=0;
  int srcp=0;
  while (srcp<srcc) {
    if ((unsigned char)src[srcp]<=0x20) { srcp++; continue; }
    const char *token=src+srcp;
    int tokenc=0;
    while ((srcp<srcc)&&((unsigned char)src[srcp]>0x20)) { srcp++; tokenc++; }
    
    #define _(tag) if ((tokenc==sizeof(#tag)-1)&&!memcmp(token,#tag,tokenc)) { \
      spritecvt->flags|=FMN_SPRITE_FLAG_##tag; \
      continue; \
    }
    FMN_FOR_EACH_SPRITE_FLAG
    #undef _
    
    else {
      fprintf(stderr,
        "%s:%d: Unexpected sprite flag '%.*s'. See FMN_SPRITE_FLAG_* in src/game/sprite/fmn_sprite.h\n",
        spritecvt->srcpath,lineno,tokenc,token
      );
      return -2;
    }
  }
  return 0;
}

/* Evaluate and apply one line.
 */
 
static int spritecvt_parse_line(struct spritecvt *spritecvt,const char *src,int srcc,int lineno) {
  int srcp=0;
  while ((srcp<srcc)&&((unsigned char)src[srcp]<=0x20)) srcp++;
  
  const char *k=src+srcp;
  int kc=0;
  while ((srcp<srcc)&&((unsigned char)src[srcp]>0x20)) { srcp++; kc++; }
  while ((srcp<srcc)&&((unsigned char)src[srcp]<=0x20)) srcp++;
  
  if ((kc==4)&&!memcmp(k,"type",4)) {
    return spritecvt_parse_type(spritecvt,src+srcp,srcc-srcp,lineno);
  }
  if ((kc==5)&&!memcmp(k,"image",5)) {
    return spritecvt_parse_image(spritecvt,src+srcp,srcc-srcp,lineno);
  }
  if ((kc==6)&&!memcmp(k,"tileid",6)) {
    return spritecvt_parse_int(&spritecvt->tileid,spritecvt,src+srcp,srcc-srcp,lineno,"tileid",0,255);
  }
  if ((kc==5)&&!memcmp(k,"xform",5)) {
    return spritecvt_parse_xform(spritecvt,src+srcp,srcc-srcp,lineno);
  }
  if ((kc==5)&&!memcmp(k,"flags",5)) {
    return spritecvt_parse_flags(spritecvt,src+srcp,srcc-srcp,lineno);
  }
  if ((kc==5)&&!memcmp(k,"layer",5)) {
    return spritecvt_parse_int(&spritecvt->layer,spritecvt,src+srcp,srcc-srcp,lineno,"layer",-128,127);
  }
  
  fprintf(stderr,"%s:%d: Unknown sprite key '%.*s'\n",spritecvt->srcpath,lineno,kc,k);
  return -2;
}

/* Decode text to object.
 */
 
int spritecvt_fmn_from_text(struct spritecvt *spritecvt) {
  if (spritecvt->tname||spritecvt->imagename) return -1;
  int srcp=0,lineno=0;
  while (srcp<spritecvt->srcc) {
    lineno++;
    const char *line=spritecvt->src+srcp;
    int linec=0;
    while ((srcp<spritecvt->srcc)&&(spritecvt->src[srcp++]!=0x0a)) linec++;
    while (linec&&((unsigned char)line[linec-1]<=0x20)) linec--;
    while (linec&&((unsigned char)line[0]<=0x20)) linec--;
    if (!linec) continue;
    
    int err=spritecvt_parse_line(spritecvt,line,linec,lineno);
    if (err<0) {
      if (err!=-2) fprintf(stderr,"%s:%d: Unspecified error.\n",spritecvt->srcpath,lineno);
      return -2;
    }
  }
  return 0;
}
