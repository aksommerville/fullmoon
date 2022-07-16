#include "fmn_genioc_internal.h"
#include <unistd.h>
#include <signal.h>

struct fmn_genioc fmn_genioc={0};

/* Signal handler.
 */
 
static void fmn_genioc_rcvsig(int sigid) {
  switch (sigid) {
    case SIGINT: if (++(fmn_genioc.sigc)>=3) {
        fprintf(stderr,"Too many unprocessed signals.\n");
        exit(1);
      } break;
  }
}

extern const struct fmn_image appicon;

/* Main.
 */

int main(int argc,char **argv) {

  signal(SIGINT,fmn_genioc_rcvsig);

  struct intf_delegate delegate={
    .userdata=0,
    .argc=argc,
    .argv=argv,
    .fbw=FMN_FBW,
    .fbh=FMN_FBH,
    .fbfmt=FMN_FBFMT,
    .fullscreen=0,//TODO configurable
    .icon_rgba=appicon.v,
    .iconw=appicon.w,
    .iconh=appicon.h,
    .title="Full Moon",
    .audio_rate=44100,//TODO configurable
    .chanc=2,//TODO configurable
    
    .close=fmn_genioc_cb_close,
    .focus=fmn_genioc_cb_focus,
    .resize=fmn_genioc_cb_resize,
    .key=fmn_genioc_cb_key,
    .text=fmn_genioc_cb_text,
    .mmotion=fmn_genioc_cb_mmotion,
    .mbutton=fmn_genioc_cb_mbutton,
    .mwheel=fmn_genioc_cb_mwheel,
    
    .pcm=fmn_genioc_cb_pcm,
    
    .connect=fmn_genioc_cb_connect,
    .disconnect=fmn_genioc_cb_disconnect,
    .event=fmn_genioc_cb_event,
  };
  if (!(fmn_genioc.intf=intf_new(&delegate))) return 1;
  
  setup();
  
  while (!fmn_genioc.terminate&&!fmn_genioc.sigc) {
  
  //TODO proper timing
    usleep(50000);
    
    if (intf_update(fmn_genioc.intf)<0) {
      fprintf(stderr,"Error updating drivers.\n");
      intf_del(fmn_genioc.intf);
      return 1;
    }
    
    loop();
  }
  
  intf_del(fmn_genioc.intf);
  fprintf(stderr,"Normal exit.\n");
  return 0;
}
