#include "fmn_genioc_internal.h"
#include <unistd.h>
#include <signal.h>

extern const struct fmn_image appicon;

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

/* Final performance report.
 */
 
static void fmn_genioc_report_performance() {
  if (fmn_genioc.framec<1) return;
  double elapsed=fmn_genioc_now_s()-fmn_genioc.starttime;
  double elapsedcpu=fmn_genioc_now_cpu_s()-fmn_genioc.starttimecpu;
  fprintf(stderr,
    "%d frames in %.03f s, average %.03f Hz. CPU=%.06f\n",
    fmn_genioc.framec,elapsed,fmn_genioc.framec/elapsed,elapsedcpu/elapsed
  );
}

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
    .audio_rate=22050,//TODO configurable
    .chanc=1,//TODO configurable
    
    .close=fmn_genioc_cb_close,
    .focus=fmn_genioc_cb_focus,
    .resize=fmn_genioc_cb_resize,
    .key=fmn_genioc_cb_key,
    .text=fmn_genioc_cb_text,
    .mmotion=fmn_genioc_cb_mmotion,
    .mbutton=fmn_genioc_cb_mbutton,
    .mwheel=fmn_genioc_cb_mwheel,
    
    .connect=fmn_genioc_cb_connect,
    .disconnect=fmn_genioc_cb_disconnect,
    .event=fmn_genioc_cb_event,
    .premapped_event=fmn_genioc_cb_premapped_event,
  };
  if (!(fmn_genioc.intf=intf_new(&delegate))) return 1;
  
  setup();
  
  fmn_genioc.frametime=1000000/FMN_GENIOC_FRAME_RATE;
  fmn_genioc.starttime=fmn_genioc_now_s();
  fmn_genioc.starttimecpu=fmn_genioc_now_cpu_s();
  fmn_genioc.nexttime=fmn_genioc_now_us();
  
  while (!fmn_genioc.terminate&&!fmn_genioc.sigc) {
  
    int64_t now=fmn_genioc_now_us();
    while (1) {
      int64_t sleeptime=fmn_genioc.nexttime-now;
      if (sleeptime<=0) {
        fmn_genioc.nexttime+=fmn_genioc.frametime;
        if (fmn_genioc.nexttime<=now) { // panic
          fmn_genioc.nexttime=now+fmn_genioc.frametime;
        }
        break;
      }
      if (sleeptime>FMN_GENIOC_SLEEP_LIMIT) { // panic
        fmn_genioc.nexttime=now+fmn_genioc.frametime;
        break;
      }
      fmn_genioc_sleep_us(sleeptime);
      now=fmn_genioc_now_us();
    }
      
    
    if (intf_update(fmn_genioc.intf)<0) {
      fprintf(stderr,"Error updating drivers.\n");
      intf_del(fmn_genioc.intf);
      return 1;
    }
    
    loop();
    
    fmn_genioc.framec++;
  }
  
  fmn_genioc_report_performance();
  intf_del(fmn_genioc.intf);
  return 0;
}
