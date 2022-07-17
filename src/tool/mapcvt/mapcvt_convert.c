#include "mapcvt.h"

/* XXX TEMP This is an ad-hoc text format that I could define in like half an hour, just to get something happening.
 * We will need fancier graphic tooling in real life.
 * That tool should output maps in an easily digestible binary format, and this tool's only job will be reencoding as C.
 */

/* Decode one line of text.
 */
 
static int mapcvt_receive_line(struct mapcvt *mapcvt,const char *src,int srcc,int lineno) {

  // Valid lines must have an even length.
  // But it could be an all-space dummy line, in which case we don't care.
  // Trim one space from the end if odd -- if the odd char is not whitespace, fail.
  if (srcc&1) {
    srcc--;
    if ((unsigned char)src[srcc]>0x20) {
      fprintf(stderr,"%s:%d: Odd line length.\n",mapcvt->srcpath,lineno);
      return -2;
    }
  }

  // Decode into a temporary row buffer before committing.
  uint8_t tmp[255];
  int tmpc=0;
  int srcp=0;
  for (;srcp<srcc;srcp+=2) {
    // Double space is noop.
    if ((src[srcp]==' ')&&(src[srcp+1]==' ')) continue;
    // Everything else must be a predefined symbol.
    if (tmpc>=sizeof(tmp)) {
      fprintf(stderr,"%s:%d: Row length exceeds %d.\n",mapcvt->srcpath,lineno,(int)sizeof(tmp));
      return -2;
    }
    #define ISSYM(str) ((src[srcp]==str[0])&&(src[srcp+1]==str[1]))
    
    /*----- defined symbols -----*/
    
         if (ISSYM(". ")) tmp[tmpc++]=0x08;
    else if (ISSYM("Xx")) tmp[tmpc++]=0x40;
    else if (ISSYM("@@")) { mapcvt->map.initx=tmpc; mapcvt->map.inity=mapcvt->map.h; tmp[tmpc++]=0x08; }
    else if (ISSYM("Bb")) tmp[tmpc++]=0x00;
    else if (ISSYM("Ww")) tmp[tmpc++]=0x0f;
    
    /*----------------------------*/
    #undef ISSYM
    else {
      fprintf(stderr,"%s:%d: Undefined symbol '%.2s' at column %d/%d.\n",mapcvt->srcpath,lineno,src+srcp,srcp,srcc);
      return -2;
    }
  }
  
  // Didn't get any tiles? It's a dummy row.
  if (!tmpc) return 0;
  
  // Hard limit of 255 rows.
  if (mapcvt->map.h>=255) {
    fprintf(stderr,"%s:%d: Map height exceeds 255 rows.\n",mapcvt->srcpath,lineno);
    return -2;
  }
  
  // First row establishes the map width, all subsequent rows must match it.
  if (!mapcvt->map.w) {
    mapcvt->map.w=tmpc;
  } else if (tmpc!=mapcvt->map.w) {
    fprintf(stderr,"%s:%d: Bad row length %d, expected %d.\n",mapcvt->srcpath,lineno,tmpc,mapcvt->map.w);
    return -2;
  }
  
  // Grow buffer and append it.
  if (mapcvt->map.h>=mapcvt->maprowa) {
    int na=mapcvt->maprowa+16;
    void *nv=realloc(mapcvt->map.v,mapcvt->map.w*na);
    if (!nv) return -1;
    mapcvt->map.v=nv;
    mapcvt->maprowa=na;
  }
  memcpy(mapcvt->map.v+mapcvt->map.w*mapcvt->map.h,tmp,tmpc);
  mapcvt->map.h++;
  
  return 0;
}

/* Decode input to a fullmoon map object.
 */
 
int mapcvt_fmn_from_text(struct mapcvt *mapcvt) {
  mapcvt->map.w=0;
  mapcvt->map.h=0;
  mapcvt->maprowa=0;
  int srcp=0,lineno=0;
  while (srcp<mapcvt->srcc) {
    lineno++;
    const char *line=mapcvt->src+srcp;
    int linec=0;
    while ((srcp<mapcvt->srcc)&&(mapcvt->src[srcp++]!=0x0a)) linec++;
    // Don't trim whitespace or comments.
    int err=mapcvt_receive_line(mapcvt,line,linec,lineno);
    if (err<0) {
      if (err!=-2) fprintf(stderr,"%s:%d: Unspecified error.\n",mapcvt->srcpath,lineno);
      return -2;
    }
  }
  if (!mapcvt->map.h) {
    fprintf(stderr,"%s: No content\n",mapcvt->srcpath);
    return -2;
  }
  if ((mapcvt->map.w%FMN_SCREENW_TILES)||(mapcvt->map.h%FMN_SCREENH_TILES)) {
    fprintf(stderr,"%s: %dx%d map is not a multiple of screen size %dx%d\n",mapcvt->srcpath,mapcvt->map.w,mapcvt->map.h,FMN_SCREENW_TILES,FMN_SCREENH_TILES);
    return -2;
  }
  return 0;
}
