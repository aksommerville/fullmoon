#include "imgcvt.h"

/* Cleanup.
 */
 
static void fmn_image_cleanup(struct fmn_image *image) {
  if (image->v) free(image->v);
}
 
static void imgcvt_cleanup(struct imgcvt *imgcvt) {
  cli_cleanup(&imgcvt->cli);
  if (imgcvt->srcpath) free(imgcvt->srcpath);
  if (imgcvt->dstpath) free(imgcvt->dstpath);
  if (imgcvt->src) free(imgcvt->src);
  if (imgcvt->dst) free(imgcvt->dst);
  png_image_cleanup(&imgcvt->png);
  fmn_image_cleanup(&imgcvt->image);
}

/* Log error.
 */
 
static int imgcvt_fail(struct imgcvt *imgcvt,const char *fmt,...) {
  fprintf(stderr,"%s: ",imgcvt->cli.exename);
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
 
static int imgcvt_main(struct imgcvt *imgcvt) {

  if (!imgcvt->dstpath) return imgcvt_fail(imgcvt,"Output path required.");
  if (!imgcvt->srcpath) return imgcvt_fail(imgcvt,"Input path required.");

  if ((imgcvt->srcc=file_read(&imgcvt->src,imgcvt->srcpath))<0) {
    fprintf(stderr,"%s: Failed to read file.\n",imgcvt->srcpath);
    return -2;
  }
  
  if (png_image_decode(&imgcvt->png,imgcvt->src,imgcvt->srcc)<0) {
    fprintf(stderr,"%s: Failed to decode as PNG.\n",imgcvt->srcpath);
    return -2;
  }
  
  int err=imgcvt_fmn_from_png(imgcvt);
  if (err<0) {
    if (err!=-2) fprintf(stderr,"%s: Failed to convert to internal format.\n",imgcvt->srcpath);
    return -2;
  }
  
  if ((err=imgcvt_encode_c(imgcvt))<0) {
    if (err!=-2) fprintf(stderr,"%s: Failed to encode converted image as C.\n",imgcvt->srcpath);
    return -2;
  }
  
  if (file_write(imgcvt->dstpath,imgcvt->dst,imgcvt->dstc)<0) {
    fprintf(stderr,"%s: Failed to write %d-byte file.\n",imgcvt->dstpath,imgcvt->dstc);
    return -2;
  }
  
  return 0;
}

/* Accessors.
 */
 
static int imgcvt_set_dstpath(struct imgcvt *imgcvt,const char *src,int srcc) {
  if (imgcvt->dstpath) return imgcvt_fail(imgcvt,"Multiple output paths.");
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (!(imgcvt->dstpath=malloc(srcc+1))) return -1;
  memcpy(imgcvt->dstpath,src,srcc);
  imgcvt->dstpath[srcc]=0;
  return 0;
}

static int imgcvt_set_srcpath(struct imgcvt *imgcvt,const char *src,int srcc) {
  if (imgcvt->srcpath) return imgcvt_fail(imgcvt,"Multiple input paths.");
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (!(imgcvt->srcpath=malloc(srcc+1))) return -1;
  memcpy(imgcvt->srcpath,src,srcc);
  imgcvt->srcpath[srcc]=0;
  return 0;
}

/* Command line.
 */
 
static int imgcvt_arg_positional(struct cli *cli,const char *src,int srcc) {
  return -1;
}

static int imgcvt_arg_option(struct cli *cli,const char *k,int kc,const char *v,int vc) {
  if (kc==1) switch (k[0]) {
    case 'o': return imgcvt_set_dstpath(IMGCVT,v,vc);
    case 'i': return imgcvt_set_srcpath(IMGCVT,v,vc);
  }
  if ((kc==7)&&!memcmp(k,"progmem",7)) {
    IMGCVT->progmem=1;
    return 0;
  }
  if ((kc==6)&&!memcmp(k,"format",6)) {
    #define _(tag) if ((vc==sizeof(#tag)-1)&&!memcmp(v,#tag,vc)) { IMGCVT->format=FMN_IMGFMT_##tag; return 0; }
    FMN_FOR_EACH_IMGFMT
    #undef _
    fprintf(stderr,"%s: Unknown image format '%.*s'\n",cli->exename,vc,v);
    return -2;
  }
  return -1;
}

static void imgcvt_help_extra(struct cli *cli) {
  fprintf(stderr,
    "  -oPATH             Output path.\n"
    "  -iPATH             Input path.\n"
    "  --progmem          Add PROGMEM declaration, for Arduino.\n"
    "  --format=NAME      "
  );
  #define _(tag) fprintf(stderr,"%s, ",#tag);
  FMN_FOR_EACH_IMGFMT
  #undef _
  fprintf(stderr,"or omit to infer from input.\n");
}

/* Main.
 */
 
int main(int argc,char **argv) {
  struct imgcvt imgcvt={0};
  struct cli_config config={
    .arg_positional=imgcvt_arg_positional,
    .arg_option=imgcvt_arg_option,
    .help_extra=imgcvt_help_extra,
  };
  if (cli_init(&imgcvt.cli,&config,argc,argv)<0) return 1;
  int status=(imgcvt_main(&imgcvt)<0)?1:0;
  imgcvt_cleanup(&imgcvt);
  return status;
}
