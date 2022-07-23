#include "tileprops.h"

/* Decode body line: 32 hex digits.
 */
 
static int tileprops_parse_body_line(struct tileprops *tileprops,const char *src,int srcc,int lineno) {
  if (srcc!=32) {
    fprintf(stderr,"%s:%d: Line must be 32 hexadecimal digits. Found length %d.\n",tileprops->srcpath,lineno,srcc);
    return -2;
  }
  
  // We'll decode all 4 banks, and even validate syntax beyond that.
  // But we only actually use the first bank, the first 16 lines of the file.
  uint8_t *dst=0;
  uint8_t *bank=0;
  switch (tileprops->y>>4) {
    case 0: bank=tileprops->props; break;
    case 1: bank=tileprops->groups; break;
    case 2: bank=tileprops->masks; break;
    case 3: bank=tileprops->priorities; break;
  }
  if (bank) dst=bank+((tileprops->y&15)<<4);
  
  int i=16;
  for (;i-->0;src+=2) {
    int hi=digit_eval(src[0]);
    if ((hi<0)||(hi>15)) {
      fprintf(stderr,"%s:%d: Expected hexadecimal digits, found '%c'\n",tileprops->srcpath,lineno,src[0]);
      return -2;
    }
    int lo=digit_eval(src[1]);
    if ((lo<0)||(lo>15)) {
      fprintf(stderr,"%s:%d: Expected hexadecimal digits, found '%c'\n",tileprops->srcpath,lineno,src[1]);
      return -2;
    }
    if (dst) {
      *dst=(hi<<4)|lo;
      dst++;
    }
  }
  return 0;
}

/* Decode text to object.
 */
 
int tileprops_fmn_from_text(struct tileprops *tileprops) {
  if (tileprops->y) return -1;
  int srcp=0,lineno=0;
  
  /* Read body.
   */
  int y=0;
  while (srcp<tileprops->srcc) {
    lineno++;
    const char *line=tileprops->src+srcp;
    int linec=0;
    while ((srcp<tileprops->srcc)&&(tileprops->src[srcp++]!=0x0a)) linec++;
    while (linec&&((unsigned char)line[linec-1]<=0x20)) linec--;
    while (linec&&((unsigned char)line[0]<=0x20)) linec--;
    if (!linec) continue;
    
    int err=tileprops_parse_body_line(tileprops,line,linec,lineno);
    if (err<0) {
      if (err!=-2) fprintf(stderr,"%s:%d: Unspecified error.\n",tileprops->srcpath,lineno);
      return -2;
    }
    tileprops->y++;
  }
  
  // Could check (y) but who cares. Short is fine.
  
  return 0;
}
