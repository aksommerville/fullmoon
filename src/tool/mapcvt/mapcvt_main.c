#include "mapcvt.h"

/* Cleanup.
 */
 
static void fmn_map_cleanup(struct fmn_map *map) {
  if (map->v) free(map->v);
  // (tilesheet,tileprops) not used
  if (map->poiv) {
    struct fmn_map_poi *poi=map->poiv;
    int i=map->poic;
    for (;i-->0;poi++) {
      if (poi->qp) free(poi->qp);
    }
    free(map->poiv);
  }
}
 
static void mapcvt_cleanup(struct mapcvt *mapcvt) {
  cli_cleanup(&mapcvt->cli);
  if (mapcvt->srcpath) free(mapcvt->srcpath);
  if (mapcvt->dstpath) free(mapcvt->dstpath);
  if (mapcvt->src) free(mapcvt->src);
  if (mapcvt->dst) free(mapcvt->dst);
  fmn_map_cleanup(&mapcvt->map);
  if (mapcvt->tilesheetname) free(mapcvt->tilesheetname);
}

/* Log error.
 */
 
static int mapcvt_fail(struct mapcvt *mapcvt,const char *fmt,...) {
  fprintf(stderr,"%s: ",mapcvt->cli.exename);
  if (fmt&&fmt[0]) {
    va_list vargs;
    va_start(vargs,fmt);
    vfprintf(stderr,fmt,vargs);
  } else {
    fprintf(stderr,"Unknown error.");
  }
  fprintf(stderr,"\n");
  return -2;
}

/* Main, with established context.
 */
 
static int mapcvt_main(struct mapcvt *mapcvt) {

  if (!mapcvt->dstpath) return mapcvt_fail(mapcvt,"Output path required.");
  if (!mapcvt->srcpath) return mapcvt_fail(mapcvt,"Input path required.");

  if ((mapcvt->srcc=file_read(&mapcvt->src,mapcvt->srcpath))<0) {
    fprintf(stderr,"%s: Failed to read file.\n",mapcvt->srcpath);
    return -2;
  }
  
  int err=mapcvt_fmn_from_text(mapcvt);
  if (err<0) {
    if (err!=-2) fprintf(stderr,"%s: Failed to decode map.\n",mapcvt->srcpath);
    return -2;
  }
  
  if ((err=mapcvt_encode_c(mapcvt))<0) {
    if (err!=-2) fprintf(stderr,"%s: Failed to encode map as C.\n",mapcvt->srcpath);
    return -2;
  }
  
  if (file_write(mapcvt->dstpath,mapcvt->dst,mapcvt->dstc)<0) {
    fprintf(stderr,"%s: Failed to write %d-byte file.\n",mapcvt->dstpath,mapcvt->dstc);
    return -2;
  }
  
  return 0;
}

/* Accessors.
 */
 
static int mapcvt_set_dstpath(struct mapcvt *mapcvt,const char *src,int srcc) {
  if (mapcvt->dstpath) return mapcvt_fail(mapcvt,"Multiple output paths.");
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (!(mapcvt->dstpath=malloc(srcc+1))) return -1;
  memcpy(mapcvt->dstpath,src,srcc);
  mapcvt->dstpath[srcc]=0;
  return 0;
}

static int mapcvt_set_srcpath(struct mapcvt *mapcvt,const char *src,int srcc) {
  if (mapcvt->srcpath) return mapcvt_fail(mapcvt,"Multiple input paths.");
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (!(mapcvt->srcpath=malloc(srcc+1))) return -1;
  memcpy(mapcvt->srcpath,src,srcc);
  mapcvt->srcpath[srcc]=0;
  return 0;
}

/* Command line.
 */
 
static int mapcvt_arg_positional(struct cli *cli,const char *src,int srcc) {
  return -1;
}

static int mapcvt_arg_option(struct cli *cli,const char *k,int kc,const char *v,int vc) {
  if (kc==1) switch (k[0]) {
    case 'o': return mapcvt_set_dstpath(MAPCVT,v,vc);
    case 'i': return mapcvt_set_srcpath(MAPCVT,v,vc);
  }
  if ((kc==7)&&!memcmp(k,"progmem",7)) {
    MAPCVT->progmem=1;
    return 0;
  }
  return -1;
}

static void mapcvt_help_extra(struct cli *cli) {
  fprintf(stderr,
    "  -oPATH             Output path.\n"
    "  -iPATH             Input path.\n"
    "  --progmem          Insert PROGMEM declaration, for Arduino.\n"
    "\n"
  );
}

/* Main.
 */
 
int main(int argc,char **argv) {
  struct mapcvt mapcvt={0};
  struct cli_config config={
    .arg_positional=mapcvt_arg_positional,
    .arg_option=mapcvt_arg_option,
    .help_extra=mapcvt_help_extra,
  };
  if (cli_init(&mapcvt.cli,&config,argc,argv)<0) return 1;
  int status=(mapcvt_main(&mapcvt)<0)?1:0;
  mapcvt_cleanup(&mapcvt);
  return status;
}
