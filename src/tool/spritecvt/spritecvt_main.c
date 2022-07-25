#include "spritecvt.h"

/* Cleanup.
 */
 
static void spritecvt_cleanup(struct spritecvt *spritecvt) {
  cli_cleanup(&spritecvt->cli);
  if (spritecvt->srcpath) free(spritecvt->srcpath);
  if (spritecvt->dstpath) free(spritecvt->dstpath);
  if (spritecvt->src) free(spritecvt->src);
  if (spritecvt->dst) free(spritecvt->dst);
  if (spritecvt->tname) free(spritecvt->tname);
  if (spritecvt->imagename) free(spritecvt->imagename);
}

/* Log error.
 */
 
static int spritecvt_fail(struct spritecvt *spritecvt,const char *fmt,...) {
  fprintf(stderr,"%s: ",spritecvt->cli.exename);
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
 
static int spritecvt_main(struct spritecvt *spritecvt) {

  if (!spritecvt->dstpath) return spritecvt_fail(spritecvt,"Output path required.");
  if (!spritecvt->srcpath) return spritecvt_fail(spritecvt,"Input path required.");

  if ((spritecvt->srcc=file_read(&spritecvt->src,spritecvt->srcpath))<0) {
    fprintf(stderr,"%s: Failed to read file.\n",spritecvt->srcpath);
    return -2;
  }
  
  int err=spritecvt_fmn_from_text(spritecvt);
  if (err<0) {
    if (err!=-2) fprintf(stderr,"%s: Failed to decode map.\n",spritecvt->srcpath);
    return -2;
  }
  
  if ((err=spritecvt_encode_c(spritecvt))<0) {
    if (err!=-2) fprintf(stderr,"%s: Failed to encode map as C.\n",spritecvt->srcpath);
    return -2;
  }
  
  if (file_write(spritecvt->dstpath,spritecvt->dst,spritecvt->dstc)<0) {
    fprintf(stderr,"%s: Failed to write %d-byte file.\n",spritecvt->dstpath,spritecvt->dstc);
    return -2;
  }
  
  return 0;
}

/* Accessors.
 */
 
static int spritecvt_set_dstpath(struct spritecvt *spritecvt,const char *src,int srcc) {
  if (spritecvt->dstpath) return spritecvt_fail(spritecvt,"Multiple output paths.");
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (!(spritecvt->dstpath=malloc(srcc+1))) return -1;
  memcpy(spritecvt->dstpath,src,srcc);
  spritecvt->dstpath[srcc]=0;
  return 0;
}

static int spritecvt_set_srcpath(struct spritecvt *spritecvt,const char *src,int srcc) {
  if (spritecvt->srcpath) return spritecvt_fail(spritecvt,"Multiple input paths.");
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (!(spritecvt->srcpath=malloc(srcc+1))) return -1;
  memcpy(spritecvt->srcpath,src,srcc);
  spritecvt->srcpath[srcc]=0;
  return 0;
}

/* Command line.
 */
 
static int spritecvt_arg_positional(struct cli *cli,const char *src,int srcc) {
  return -1;
}

static int spritecvt_arg_option(struct cli *cli,const char *k,int kc,const char *v,int vc) {
  if (kc==1) switch (k[0]) {
    case 'o': return spritecvt_set_dstpath(MAPCVT,v,vc);
    case 'i': return spritecvt_set_srcpath(MAPCVT,v,vc);
  }
  return -1;
}

static void spritecvt_help_extra(struct cli *cli) {
  fprintf(stderr,
    "  -oPATH             Output path.\n"
    "  -iPATH             Input path.\n"
    "\n"
  );
}

/* Main.
 */
 
int main(int argc,char **argv) {
  struct spritecvt spritecvt={0};
  struct cli_config config={
    .arg_positional=spritecvt_arg_positional,
    .arg_option=spritecvt_arg_option,
    .help_extra=spritecvt_help_extra,
  };
  if (cli_init(&spritecvt.cli,&config,argc,argv)<0) return 1;
  int status=(spritecvt_main(&spritecvt)<0)?1:0;
  spritecvt_cleanup(&spritecvt);
  return status;
}
