#include "songcvt.h"

/* Cleanup.
 */
 
static void songcvt_cleanup(struct songcvt *songcvt) {
  cli_cleanup(&songcvt->cli);
  if (songcvt->srcpath) free(songcvt->srcpath);
  if (songcvt->dstpath) free(songcvt->dstpath);
  if (songcvt->src) free(songcvt->src);
  if (songcvt->dst) free(songcvt->dst);
  encoder_cleanup(&songcvt->bin);
}

/* Log error.
 */
 
static int songcvt_fail(struct songcvt *songcvt,const char *fmt,...) {
  fprintf(stderr,"%s: ",songcvt->cli.exename);
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
 
static int songcvt_main(struct songcvt *songcvt) {
  int err;

  if (!songcvt->dstpath) return songcvt_fail(songcvt,"Output path required.");
  if (!songcvt->srcpath) return songcvt_fail(songcvt,"Input path required.");

  if ((songcvt->srcc=file_read(&songcvt->src,songcvt->srcpath))<0) {
    fprintf(stderr,"%s: Failed to read file.\n",songcvt->srcpath);
    return -2;
  }
  
  if ((err=songcvt_read_adjust(songcvt))<0) {
    if (err!=-2) fprintf(stderr,"%s: Error processing adjustments file.\n",songcvt->srcpath);
    return -2;
  }
  
  if ((err=songcvt_minisyni_from_midi(songcvt))<0) {
    if (err!=-2) fprintf(stderr,"%s: Failed to decode MIDI file.\n",songcvt->srcpath);
    return -2;
  }
  
  if ((err=songcvt_encode_c(songcvt))<0) {
    if (err!=-2) fprintf(stderr,"%s: Failed to encode song as C.\n",songcvt->srcpath);
    return -2;
  }
  
  if (file_write(songcvt->dstpath,songcvt->dst,songcvt->dstc)<0) {
    fprintf(stderr,"%s: Failed to write %d-byte file.\n",songcvt->dstpath,songcvt->dstc);
    return -2;
  }
  
  return 0;
}

/* Accessors.
 */
 
static int songcvt_set_dstpath(struct songcvt *songcvt,const char *src,int srcc) {
  if (songcvt->dstpath) return songcvt_fail(songcvt,"Multiple output paths.");
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (!(songcvt->dstpath=malloc(srcc+1))) return -1;
  memcpy(songcvt->dstpath,src,srcc);
  songcvt->dstpath[srcc]=0;
  return 0;
}

static int songcvt_set_srcpath(struct songcvt *songcvt,const char *src,int srcc) {
  if (songcvt->srcpath) return songcvt_fail(songcvt,"Multiple input paths.");
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  if (!(songcvt->srcpath=malloc(srcc+1))) return -1;
  memcpy(songcvt->srcpath,src,srcc);
  songcvt->srcpath[srcc]=0;
  return 0;
}

/* Command line.
 */
 
static int songcvt_arg_positional(struct cli *cli,const char *src,int srcc) {
  return -1;
}

static int songcvt_arg_option(struct cli *cli,const char *k,int kc,const char *v,int vc) {
  if (kc==1) switch (k[0]) {
    case 'o': return songcvt_set_dstpath(SONGCVT,v,vc);
    case 'i': return songcvt_set_srcpath(SONGCVT,v,vc);
  }
  if ((kc==7)&&!memcmp(k,"progmem",7)) {
    SONGCVT->progmem=1;
    return 0;
  }
  return -1;
}

static void songcvt_help_extra(struct cli *cli) {
  fprintf(stderr,
    "  -oPATH             Output path (minisyni).\n"
    "  -iPATH             Input path (MIDI).\n"
    "  --progmem          Insert PROGMEM declaration, for Arduino.\n"
    "\n"
    "If a file 'foo.adjust' exists for an input 'foo.mid', we read preferences from it:\n"
    "  debug # Dump input file metadata.\n"
    "  MTrk=0 : chid=0 pid=0 # 'CRITERIA : ACTIONS', eg change all notes on track 0 to channel 0.\n"
    "\n"
  );
}

/* Main.
 */
 
int main(int argc,char **argv) {
  struct songcvt songcvt={0};
  struct cli_config config={
    .arg_positional=songcvt_arg_positional,
    .arg_option=songcvt_arg_option,
    .help_extra=songcvt_help_extra,
  };
  if (cli_init(&songcvt.cli,&config,argc,argv)<0) return 1;
  int status=(songcvt_main(&songcvt)<0)?1:0;
  songcvt_cleanup(&songcvt);
  return status;
}
