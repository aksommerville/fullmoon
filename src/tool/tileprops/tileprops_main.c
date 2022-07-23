#include "tileprops.h"

/* Cleanup.
 */
 
static void tileprops_cleanup(struct tileprops *tileprops) {
  cli_cleanup(&tileprops->cli);
  if (tileprops->srcpath) free(tileprops->srcpath);
  if (tileprops->dstpath) free(tileprops->dstpath);
  if (tileprops->src) free(tileprops->src);
  if (tileprops->dst) free(tileprops->dst);
}

/* Log error.
 */
 
static int tileprops_fail(struct tileprops *tileprops,const char *fmt,...) {
  fprintf(stderr,"%s: ",tileprops->cli.exename);
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
 
static int tileprops_main(struct tileprops *tileprops) {

  if (!tileprops->dstpath) return tileprops_fail(tileprops,"Output path required.");
  if (!tileprops->srcpath) return tileprops_fail(tileprops,"Input path required.");

  if ((tileprops->srcc=file_read(&tileprops->src,tileprops->srcpath))<0) {
    fprintf(stderr,"%s: Failed to read file.\n",tileprops->srcpath);
    return -2;
  }
  
  int err=tileprops_fmn_from_text(tileprops);
  if (err<0) {
    if (err!=-2) fprintf(stderr,"%s: Failed to decode map.\n",tileprops->srcpath);
    return -2;
  }
  
  if ((err=tileprops_encode_c(tileprops))<0) {
    if (err!=-2) fprintf(stderr,"%s: Failed to encode map as C.\n",tileprops->srcpath);
    return -2;
  }
  
  if (file_write(tileprops->dstpath,tileprops->dst,tileprops->dstc)<0) {
    fprintf(stderr,"%s: Failed to write %d-byte file.\n",tileprops->dstpath,tileprops->dstc);
    return -2;
  }
  
  return 0;
}

/* Accessors.
 */
 
static int tileprops_set_dstpath(struct tileprops *tileprops,const char *src,int srcc) {
  if (tileprops->dstpath) return tileprops_fail(tileprops,"Multiple output paths.");
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (!(tileprops->dstpath=malloc(srcc+1))) return -1;
  memcpy(tileprops->dstpath,src,srcc);
  tileprops->dstpath[srcc]=0;
  return 0;
}

static int tileprops_set_srcpath(struct tileprops *tileprops,const char *src,int srcc) {
  if (tileprops->srcpath) return tileprops_fail(tileprops,"Multiple input paths.");
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (!(tileprops->srcpath=malloc(srcc+1))) return -1;
  memcpy(tileprops->srcpath,src,srcc);
  tileprops->srcpath[srcc]=0;
  return 0;
}

/* Command line.
 */
 
static int tileprops_arg_positional(struct cli *cli,const char *src,int srcc) {
  return -1;
}

static int tileprops_arg_option(struct cli *cli,const char *k,int kc,const char *v,int vc) {
  if (kc==1) switch (k[0]) {
    case 'o': return tileprops_set_dstpath(MAPCVT,v,vc);
    case 'i': return tileprops_set_srcpath(MAPCVT,v,vc);
  }
  return -1;
}

static void tileprops_help_extra(struct cli *cli) {
  fprintf(stderr,
    "  -oPATH             Output path.\n"
    "  -iPATH             Input path.\n"
    "\n"
  );
}

/* Main.
 */
 
int main(int argc,char **argv) {
  struct tileprops tileprops={0};
  struct cli_config config={
    .arg_positional=tileprops_arg_positional,
    .arg_option=tileprops_arg_option,
    .help_extra=tileprops_help_extra,
  };
  if (cli_init(&tileprops.cli,&config,argc,argv)<0) return 1;
  int status=(tileprops_main(&tileprops)<0)?1:0;
  tileprops_cleanup(&tileprops);
  return status;
}
