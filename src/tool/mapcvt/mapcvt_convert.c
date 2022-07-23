#include "mapcvt.h"

/* POI type names.
 */
 
static int mapcvt_eval_poi_q0(uint8_t *dst,const char *src,int srcc) {
  #define _(tag) if ((srcc==sizeof(#tag)-1)&&!memcmp(src,#tag,srcc)) { *dst=FMN_POI_##tag; return 0; }
  _(START)
  _(DOOR)
  _(SPRITE)
  _(TREADLE)
  _(VISIBILITY)
  _(PROXIMITY)
  _(EDGE_DOOR)
  #undef _
  return -1;
}

/* Compare POI for sorting.
 */
 
static int mapcvt_poicmp(const void *_a,const void *_b) {
  const struct fmn_map_poi *a=_a,*b=_b;
  if (a->y<b->y) return -1;
  if (a->y>b->y) return 1;
  if (a->x<b->x) return -1;
  if (a->x>b->x) return 1;
  return 0;
}

/* Validate POI.
 */
 
static int mapcvt_poi_validate(struct mapcvt *mapcvt,const struct fmn_map_poi *poi) {
  if (poi->qp) {
    const char *src=poi->qp;
    int srcc=0; while (src[srcc]) srcc++;
    if ((srcc<1)||(srcc>255)) return -1;
    if ((src[0]>='0')&&(src[0]<='9')) return -1;
    for (;srcc-->0;src++) {
      if ((*src>='a')&&(*src<='z')) continue;
      if ((*src>='A')&&(*src<='Z')) continue;
      if ((*src>='0')&&(*src<='9')) continue;
      if (*src=='_') continue;
      return -1;
    }
  }
  switch (poi->q[0]) {
    case FMN_POI_START: if (poi->q[1]||poi->q[2]||poi->q[3]||poi->qp) return -1; break;
    case FMN_POI_DOOR: {
        if (poi->q[3]) return -1;
        if (!poi->qp) return -1;
      } break;
    case FMN_POI_SPRITE: if (!poi->qp) return -1; break;
    case FMN_POI_TREADLE: if (!poi->qp) return -1; break;
    case FMN_POI_VISIBILITY: if (!poi->qp) return -1; break;
    case FMN_POI_PROXIMITY: if (!poi->qp) return -1; break;
    case FMN_POI_EDGE_DOOR: {
        if (poi->q[3]) return -1;
        if (!poi->qp) return -1;
      } break;
  }
  return 0;
}

/* Decode POI.
 */
 
static int mapcvt_decode_poi(struct fmn_map_poi *poi,struct mapcvt *mapcvt,const char *src,int srcc,int lineno) {
  const char *token;
  int srcp=0,tokenc,n;
  while ((srcp<srcc)&&((unsigned char)src[srcp]<=0x20)) srcp++;
  
  token=src+srcp;
  tokenc=0;
  while ((srcp<srcc)&&((unsigned char)src[srcp]>0x20)) { srcp++; tokenc++; }
  while ((srcp<srcc)&&((unsigned char)src[srcp]<=0x20)) srcp++;
  if ((int_eval(&n,token,tokenc)<2)||(n<0)||(n>255)) {
    fprintf(stderr,"%s:%d: Expected integer in 0..255 for X, found '%.*s'\n",mapcvt->srcpath,lineno,tokenc,token);
    return -2;
  }
  poi->x=n;
  
  token=src+srcp;
  tokenc=0;
  while ((srcp<srcc)&&((unsigned char)src[srcp]>0x20)) { srcp++; tokenc++; }
  while ((srcp<srcc)&&((unsigned char)src[srcp]<=0x20)) srcp++;
  if ((int_eval(&n,token,tokenc)<2)||(n<0)||(n>255)) {
    fprintf(stderr,"%s:%d: Expected integer in 0..255 for Y, found '%.*s'\n",mapcvt->srcpath,lineno,tokenc,token);
    return -2;
  }
  poi->y=n;
  
  token=src+srcp;
  tokenc=0;
  while ((srcp<srcc)&&((unsigned char)src[srcp]>0x20)) { srcp++; tokenc++; }
  while ((srcp<srcc)&&((unsigned char)src[srcp]<=0x20)) srcp++;
  if (mapcvt_eval_poi_q0(poi->q+0,token,tokenc)<0) {
    fprintf(stderr,"%s:%d: Expected POI type name, found '%.*s' (hint: FMN_POI_* in src/game/fullmoon.h)\n",mapcvt->srcpath,lineno,tokenc,token);
    return -2;
  }
  
  int qi=1;
  for (;qi<4;qi++) {
    token=src+srcp;
    tokenc=0;
    while ((srcp<srcc)&&((unsigned char)src[srcp]>0x20)) { srcp++; tokenc++; }
    while ((srcp<srcc)&&((unsigned char)src[srcp]<=0x20)) srcp++;
    if (!tokenc) break;
    if ((int_eval(&n,token,tokenc)<2)||(n<0)||(n>255)) {
      fprintf(stderr,"%s:%d: Expected integer in 0..255 for q[%d], found '%.*s'\n",mapcvt->srcpath,lineno,qi,tokenc,token);
      return -2;
    }
    poi->q[qi]=n;
  }
  
  token=src+srcp;
  tokenc=srcc-srcp;
  if (tokenc) {
    if (!(poi->qp=malloc(tokenc+1))) return -1;
    memcpy(poi->qp,token,tokenc);
    ((char*)poi->qp)[tokenc]=0;
  }
  
  return 0;
}

/* Decode header line.
 */
 
static int mapcvt_fmn_text_header_line(struct mapcvt *mapcvt,const char *line,int linec,int lineno) {
  int linep=0;
  while ((linep<linec)&&((unsigned char)line[linep]<=0x20)) linep++;

  const char *k=line+linep;
  int kc=0;
  while ((linep<linec)&&((unsigned char)line[linep]!='=')) { linep++; kc++; }
  while (kc&&((unsigned char)k[kc-1]<=0x20)) kc--;
  if (!kc) return -1;
  
  if ((linep>=linec)||(line[linep]!='=')) {
    fprintf(stderr,"%s:%d: Expected '=' after header key '%.*s'.\n",mapcvt->srcpath,lineno,kc,k);
    return -2;
  }
  linep++;
  while ((linep<linec)&&((unsigned char)line[linep]<=0x20)) linep++;
  
  if ((kc==9)&&!memcmp(k,"tilesheet",9)) {
    if (mapcvt->tilesheetname) {
      fprintf(stderr,"%s:%d: Duplicate 'tilesheet'\n",mapcvt->srcpath,lineno);
      return -2;
    }
    if (!(mapcvt->tilesheetname=malloc(linec-linep+1))) return -1;
    memcpy(mapcvt->tilesheetname,line+linep,linec-linep);
    mapcvt->tilesheetname[linec-linep]=0;
    return 0;
  }
  
  if ((kc==4)&&!memcmp(k,"size",4)) {
    if (mapcvt->map.w) {
      fprintf(stderr,"%s:%d: Duplicate 'size'\n",mapcvt->srcpath,lineno);
      return -2;
    }
    int w,h;
    const char *token=line+linep;
    int tokenc=0;
    while ((linep<linec)&&((unsigned char)line[linep++]>0x20)) tokenc++;
    while ((linep<linec)&&((unsigned char)line[linep]<=0x20)) linep++;
    if ((int_eval(&w,token,tokenc)<2)||(w<1)||(w>255)||(w%9)) {
      fprintf(stderr,"%s:%d: Expected multiple of 9 in 9..252, found '%.*s'\n",mapcvt->srcpath,lineno,tokenc,token);
      return -2;
    }
    token=line+linep;
    tokenc=0;
    while ((linep<linec)&&((unsigned char)line[linep++]>0x20)) tokenc++;
    while ((linep<linec)&&((unsigned char)line[linep]<=0x20)) linep++;
    if ((int_eval(&h,token,tokenc)<2)||(h<1)||(h>255)||(h%5)) {
      fprintf(stderr,"%s:%d: Expected multiple of 5 in 5..255, found '%.*s'\n",mapcvt->srcpath,lineno,tokenc,token);
      return -2;
    }
    if (linep<linec) {
      fprintf(stderr,"%s:%d: Unexpected tokens after size: '%.*s'\n",mapcvt->srcpath,lineno,linec-linep,line+linep);
      return -2;
    }
    mapcvt->map.w=w;
    mapcvt->map.h=h;
    return 0;
  }
  
  if ((kc==3)&&!memcmp(k,"poi",3)) {
    if (mapcvt->map.poic>=0xffff) {
      fprintf(stderr,"%s:%d: Too many POI, limit 65535. How the... really?\n",mapcvt->srcpath,lineno);
      return -2;
    }
    if (mapcvt->map.poic>=mapcvt->mappoia) {
      int na=mapcvt->mappoia+32;
      void *nv=realloc(mapcvt->map.poiv,sizeof(struct fmn_map_poi)*na);
      if (!nv) return -1;
      mapcvt->map.poiv=nv;
      mapcvt->mappoia=na;
    }
    struct fmn_map_poi *poi=mapcvt->map.poiv+mapcvt->map.poic;
    memset(poi,0,sizeof(struct fmn_map_poi));

    int err=mapcvt_decode_poi(poi,mapcvt,line+linep,linec-linep,lineno);
    if (err<0) {
      if (err!=-2) fprintf(stderr,"%s:%d: Failed to decode POI content.\n",mapcvt->srcpath,lineno);
      return -2;
    }
  
    mapcvt->map.poic++;
    return 0;
  }
  
  fprintf(stderr,"%s:%d: Unexpected header key '%.*s'\n",mapcvt->srcpath,lineno,kc,k);
  return -2;
}

/* Decode body line.
 * Caller must validate lengths.
 */
 
static int mapcvt_parse_body_line(struct mapcvt *mapcvt,uint8_t *dst,const char *src,int lineno) {
  int i=mapcvt->map.w;
  for (;i-->0;dst++,src+=2) {
    int hi=digit_eval(src[0]);
    if ((hi<0)||(hi>15)) {
      fprintf(stderr,"%s:%d: Expected hexadecimal digits, found '%c'\n",mapcvt->srcpath,lineno,src[0]);
      return -2;
    }
    int lo=digit_eval(src[1]);
    if ((lo<0)||(lo>15)) {
      fprintf(stderr,"%s:%d: Expected hexadecimal digits, found '%c'\n",mapcvt->srcpath,lineno,src[1]);
      return -2;
    }
    *dst=(hi<<4)|lo;
  }
  return 0;
}

/* Decode text to object.
 */
 
int mapcvt_fmn_from_text(struct mapcvt *mapcvt) {
  if (mapcvt->map.w||mapcvt->map.h||mapcvt->map.poic) return -1;
  int srcp=0,lineno=0;
  
  /* Read header.
   */
  while (srcp<mapcvt->srcc) {
    lineno++;
    const char *line=mapcvt->src+srcp;
    int linec=0,cmt=0;
    while (srcp<mapcvt->srcc) {
      if (mapcvt->src[srcp]=='#') cmt=1;
      else if (mapcvt->src[srcp]==0x0a) { srcp++; break; }
      else if (!cmt) linec++;
      srcp++;
    }
    while (linec&&((unsigned char)line[linec-1]<=0x20)) linec--;
    while (linec&&((unsigned char)line[0]<=0x20)) { line++; linec--; }
    if (!linec) continue;
    if ((linec==10)&&!memcmp(line,"BEGIN_BODY",10)) break;
    int err=mapcvt_fmn_text_header_line(mapcvt,line,linec,lineno);
    if (err<0) {
      if (err!=-2) fprintf(stderr,"%s:%d: Unspecified error.\n",mapcvt->srcpath,lineno);
      return -2;
    }
  }
  
  /* Validate header.
   */
  if (
    (mapcvt->map.w<1)||(mapcvt->map.w%FMN_SCREENW_TILES)||
    (mapcvt->map.h<1)||(mapcvt->map.h%FMN_SCREENH_TILES)
  ) {
    fprintf(stderr,
      "%s: Invalid dimensions %dx%d, must be nonzero multiple of %dx%d no more than 255.\n",
      mapcvt->srcpath,mapcvt->map.w,mapcvt->map.h,
      FMN_SCREENW_TILES,FMN_SCREENH_TILES
    );
    return -2;
  }
  if (!(mapcvt->map.v=malloc(mapcvt->map.w*mapcvt->map.h))) return -1;
  if (!mapcvt->tilesheetname) {
    fprintf(stderr,"%s: Tilesheet name required.\n",mapcvt->srcpath);
    return -2;
  }
  const struct fmn_map_poi *poi=mapcvt->map.poiv;
  int i=mapcvt->map.poic;
  for (;i-->0;poi++) {
    int err=mapcvt_poi_validate(mapcvt,poi);
    if (err<0) {
      if (err!=-2) fprintf(stderr,
        "%s: Invalid POI: x=%d,y=%d,q=[%d,%d,%d,%d],qp=%s\n",
        mapcvt->srcpath,poi->x,poi->y,poi->q[0],poi->q[1],poi->q[2],poi->q[3],poi->qp
      );
      return -2;
    }
  }
  qsort(mapcvt->map.poiv,mapcvt->map.poic,sizeof(struct fmn_map_poi),mapcvt_poicmp);
  
  /* Read body.
   */
  int y=0;
  while (srcp<mapcvt->srcc) {
    lineno++;
    if (y>=mapcvt->map.h) {
      fprintf(stderr,"%s:%d: Body exceeds %d rows.\n",mapcvt->srcpath,lineno,mapcvt->map.h);
      return -2;
    }
    const char *line=mapcvt->src+srcp;
    int linec=0;
    while ((srcp<mapcvt->srcc)&&(mapcvt->src[srcp++]!=0x0a)) linec++;
    if (linec&1) {
      fprintf(stderr,"%s:%d: Odd line length\n",mapcvt->srcpath,lineno);
      return -2;
    }
    if (linec!=mapcvt->map.w<<1) {
      fprintf(stderr,"%s:%d: Expected %d columns, found %d.\n",mapcvt->srcpath,lineno,mapcvt->map.w,linec>>1);
      return -2;
    }
    int err=mapcvt_parse_body_line(mapcvt,mapcvt->map.v+y*mapcvt->map.w,line,lineno);
    if (err<0) {
      if (err!=-2) fprintf(stderr,"%s:%d: Unspecified error.\n",mapcvt->srcpath,lineno);
      return -2;
    }
    y++;
  }
  if (y<mapcvt->map.h) {
    fprintf(stderr,"%s: Found %d rows, expected %d.\n",mapcvt->srcpath,y,mapcvt->map.h);
    return -2;
  }
  
  return 0;
}
