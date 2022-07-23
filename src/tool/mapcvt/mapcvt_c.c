#include "mapcvt.h"

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

/* Encode declarations of all external objects.
 */
 
static int mapcvt_encode_linked_objects(struct encoder *dst,struct mapcvt *mapcvt) {

  if (encode_fmt(dst,"extern const struct fmn_image %s;\n",mapcvt->tilesheetname)<0) return -1;
  if (encode_fmt(dst,"extern const uint8_t %s_props[256];\n",mapcvt->tilesheetname)<0) return -1;
  
  const struct fmn_map_poi *poi=mapcvt->map.poiv;
  int i=mapcvt->map.poic;
  for (;i-->0;poi++) {
    if (!poi->qp) continue;
    switch (poi->q[0]) {
      case FMN_POI_DOOR: if (encode_fmt(dst,"extern const struct fmn_map %s;\n",poi->qp)<0) return -1; break;
      case FMN_POI_SPRITE: if (encode_fmt(dst,"extern const struct fmn_sprdef %s;\n",poi->qp)<0) return -1; break;
      // exact signature for functions doesn't matter:
      case FMN_POI_TREADLE: if (encode_fmt(dst,"void %s();\n",poi->qp)<0) return -1; break;
      case FMN_POI_VISIBILITY: if (encode_fmt(dst,"void %s();\n",poi->qp)<0) return -1; break;
      case FMN_POI_PROXIMITY: if (encode_fmt(dst,"void %s();\n",poi->qp)<0) return -1; break;
      case FMN_POI_EDGE_DOOR: if (encode_fmt(dst,"extern const struct fmn_map %s;\n",poi->qp)<0) return -1; break;
      default: {
          // unknown type is an error; we don't know how to type the external object.
          // note that if no extern is present for this poi, unknown type is just fine.
          fprintf(stderr,"Unknown POI type 0x%02x.\n",poi->q[0]);
          return -2;
        }
    }
  }
  
  return 0;
}

/* Encode one POI.
 */
 
static int mapcvt_encode_poi(struct encoder *dst,struct mapcvt *mapcvt,const struct fmn_map_poi *poi) {
  if (encode_fmt(dst,"  {\n")<0) return -1;
  if (encode_fmt(dst,"    .x=%d,\n",poi->x)<0) return -1;
  if (encode_fmt(dst,"    .y=%d,\n",poi->y)<0) return -1;
  if (encode_fmt(dst,"    .q={%d,%d,%d,%d},\n",poi->q[0],poi->q[1],poi->q[2],poi->q[3])<0) return -1;
  if (poi->qp) {
    if (encode_fmt(dst,"    .qp=(void*)&%s,\n",poi->qp)<0) return -1; // sic no quotes; it's a C identifier
  }
  if (encode_fmt(dst,"  },\n")<0) return -1;
  return 0;
}

/* Encode in context.
 */
 
static int mapcvt_encode_c_to_encoder(struct encoder *dst,struct mapcvt *mapcvt) {
  int err;

  if (encode_fmt(dst,"#include <stdint.h>\n")<0) return -1;
  if (encode_fmt(dst,"#include \"game/fullmoon.h\"\n")<0) return -1;
  if ((err=mapcvt_encode_linked_objects(dst,mapcvt))<0) return err;
  
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
  
  if (encode_fmt(dst,"const struct fmn_map_poi %.*s_POI[]={\n",namec,name)<0) return -1;
  const struct fmn_map_poi *poi=mapcvt->map.poiv;
  for (i=mapcvt->map.poic;i-->0;poi++) {
    if ((err=mapcvt_encode_poi(dst,mapcvt,poi))<0) return err;
  }
  if (encode_fmt(dst,"};\n")<0) return -1;
  
  if (encode_fmt(dst,"const struct fmn_map %.*s={\n",namec,name)<0) return -1;
  if (encode_fmt(dst,"  .v=(void*)%.*s_STORAGE,\n",namec,name)<0) return -1;
  if (encode_fmt(dst,"  .w=%d,\n",mapcvt->map.w)<0) return -1;
  if (encode_fmt(dst,"  .h=%d,\n",mapcvt->map.h)<0) return -1;
  if (encode_fmt(dst,"  .tilesheet=&%s,\n",mapcvt->tilesheetname)<0) return -1;
  if (encode_fmt(dst,"  .tileprops=%s_props,\n",mapcvt->tilesheetname)<0) return -1;
  if (encode_fmt(dst,"  .poic=%d,\n",mapcvt->map.poic)<0) return -1;
  if (encode_fmt(dst,"  .poiv=(void*)%.*s_POI,\n",namec,name)<0) return -1;
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
