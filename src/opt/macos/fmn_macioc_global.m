#include "fmn_macioc_internal.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

struct fmn_macioc fmn_macioc={0};

/* Log to a text file. Will work even if the TTY is unset.
 */
#if 0 // I'll just leave this here in case we need it again...
static void fmn_macioc_surelog(const char *fmt,...) {
  va_list vargs;
  va_start(vargs,fmt);
  char message[256];
  int messagec=vsnprintf(message,sizeof(message),fmt,vargs);
  if ((messagec<0)||(messagec>=sizeof(message))) {
    messagec=snprintf(message,sizeof(message),"(unable to generate message)");
  }
  int f=open("/Users/andy/proj/fullmoon/surelog",O_WRONLY|O_APPEND|O_CREAT,0666);
  if (f<0) return;
  int err=write(f,message,messagec);
  close(f);
}

#define SURELOG(fmt,...) fmn_macioc_surelog("%d:%s:%d:%s: "fmt"\n",(int)time(0),__FILE__,__LINE__,__func__,##__VA_ARGS__);
#endif

/* Reopen TTY.
 */
 
static int fmn_macioc_reopen_tty(const char *path) {
  int fd=open(path,O_RDWR);
  if (fd<0) return -1;
  dup2(fd,STDIN_FILENO);
  dup2(fd,STDOUT_FILENO);
  dup2(fd,STDERR_FILENO);
  close(fd);
  return 0;
}

/* First pass through argv.
 */

static int fmn_macioc_argv_prerun(int argc,char **argv) {

  // argv[0] will have the full path to the executable. A bit excessive.
  if ((argc>=1)&&argv[0]) {
    char *src=argv[0];
    char *base=src;
    for (;*src;src++) if (*src=='/') base=src+1;
    argv[0]=base;
  }

  int argp;
  for (argp=1;argp<argc;argp++) {
    const char *k=argv[argp];
    if (!k) continue;
    if ((k[0]!='-')||(k[1]!='-')||!k[2]) continue;
    k+=2;
    int kc=0;
    while (k[kc]&&(k[kc]!='=')) kc++;
    const char *v=k+kc;
    int vc=0;
    if (v[0]=='=') {
      v++;
      while (v[vc]) vc++;
    }

    if ((kc==10)&&!memcmp(k,"reopen-tty",10)) {
      if (fmn_macioc_reopen_tty(v)<0) return -1;
      argv[argp]="";

    } else if ((kc==5)&&!memcmp(k,"chdir",5)) {
      if (chdir(v)<0) return -1;
      argv[argp]="";

    }
  }
  return 0;
}

/* Configure.
 */

static int fmn_macioc_configure(int argc,char **argv) {
  int argp;

  if (fmn_macioc_argv_prerun(argc,argv)<0) return -1;
  fmn_macioc.argc=argc;
  fmn_macioc.argv=argv;

  #if FMN_USE_inmap
    fmn_macioc.inmap=fmn_inmap_new();
    //TODO config file
  #endif

  return 0;
}

/* Main.
 */

int main(int argc,char **argv) {

  if (fmn_macioc.init) return 1;
  memset(&fmn_macioc,0,sizeof(struct fmn_macioc));
  fmn_macioc.init=1;

  if (fmn_macioc_configure(argc,argv)<0) return 1;

  return NSApplicationMain(argc,(const char**)argv);
}

/* Abort.
 */
 
void fmn_macioc_abort(const char *fmt,...) {
  if (fmt&&fmt[0]) {
    va_list vargs;
    va_start(vargs,fmt);
    char msg[256];
    int msgc=vsnprintf(msg,sizeof(msg),fmt,vargs);
    if ((msgc<0)||(msgc>=sizeof(msg))) msgc=0;
    fprintf(stderr,"%.*s\n",msgc,msg);
  }
  [NSApplication.sharedApplication terminate:nil];
  fprintf(stderr,"!!! [NSApplication.sharedApplication terminate:nil] did not terminate execution. Using exit() instead !!!\n");
  exit(1);
}

/* Start up, after NSApplication does its thing.
 */
 
static int fmn_macioc_init() {

  struct intf_delegate delegate={
    .userdata=0,
    .argc=fmn_macioc.argc,
    .argv=fmn_macioc.argv,
    .fbw=FMN_FBW,
    .fbh=FMN_FBH,
    .fbfmt=FMN_FBFMT,
    .fullscreen=0,//TODO configurable
    // icon_* are managed by the bundle, not here
    .title="Full Moon",
    .audio_rate=44100,//TODO configurable
    .chanc=2,//TODO configurable
    
    .close=fmn_macioc_cb_close,
    .focus=fmn_macioc_cb_focus,
    .resize=fmn_macioc_cb_resize,
    .key=fmn_macioc_cb_key,
    .text=fmn_macioc_cb_text,
    .mmotion=fmn_macioc_cb_mmotion,
    .mbutton=fmn_macioc_cb_mbutton,
    .mwheel=fmn_macioc_cb_mwheel,
    
    .pcm=fmn_macioc_cb_pcm,
    
    .connect=fmn_macioc_cb_connect,
    .disconnect=fmn_macioc_cb_disconnect,
    .event=fmn_macioc_cb_event,
    .premapped_event=fmn_macioc_cb_premapped_event,
  };
  if (!(fmn_macioc.intf=intf_new(&delegate))) return -1;
  
  setup();

  return 0;
}

/* Entry points from game.
 */

// Not necessary; we do the initting and updating out-of-band.
void fmn_platform_init() {}
void fmn_platform_update() {}

void fmn_platform_send_framebuffer(const void *fb) {
  if (fmn_macioc.intf) video_driver_swap(fmn_macioc.intf->video,fb);
}

uint16_t fmn_platform_read_input() {
  return fmn_macioc.input
  #if FMN_USE_inmap
    |fmn_inmap_get_state(fmn_macioc.inmap)
  #endif
  ;
}

/* Final performance report.
 */
 
static void fmn_macioc_report_performance() {
  if (fmn_macioc.framec<1) return;
  double elapsed=fmn_macioc_now_s()-fmn_macioc.starttime;
  double elapsedcpu=fmn_macioc_now_cpu_s()-fmn_macioc.starttimecpu;
  fprintf(stderr,
    "%d frames in %.03f s, average %.03f Hz. CPU=%.06f\n",
    fmn_macioc.framec,elapsed,fmn_macioc.framec/elapsed,elapsedcpu/elapsed
  );
}

@implementation AKAppDelegate

/* Main loop.
 * This runs on a separate thread.
 */

-(void)mainLoop:(id)ignore {

  fmn_macioc.frametime=1000000/FMN_MACIOC_FRAME_RATE;
  fmn_macioc.starttime=fmn_macioc_now_s();
  fmn_macioc.starttimecpu=fmn_macioc_now_cpu_s();
  fmn_macioc.nexttime=fmn_macioc_now_us();
  
  while (1) {

    if (fmn_macioc.terminate) break;

    int64_t now=fmn_macioc_now_us();
    while (1) {
      int64_t sleeptime=fmn_macioc.nexttime-now;
      if (sleeptime<=0) {
        fmn_macioc.nexttime+=fmn_macioc.frametime;
        if (fmn_macioc.nexttime<=now) { // panic
          fmn_macioc.nexttime=now+fmn_macioc.frametime;
        }
        break;
      }
      if (sleeptime>FMN_MACIOC_SLEEP_LIMIT) { // panic
        fmn_macioc.nexttime=now+fmn_macioc.frametime;
        break;
      }
      fmn_macioc_sleep_us(sleeptime);
      now=fmn_macioc_now_us();
    }

    if (fmn_macioc.terminate) break;

    if (fmn_macioc.update_in_progress) {
      //fmn_log(MACIOC,TRACE,"Dropping frame due to update still running.");
      continue;
    }

    /* With 'waitUntilDone:0', we will always be on manual timing.
     * I think that's OK. And window resizing is much more responsive this way.
     * Update:
     *   !!! After upgrading from 10.11 to 10.13, all the timing got fucked.
     *   Switching to 'waitUntilDone:1' seems to fix it.
     *   If the only problem that way in 10.11 was choppy window resizing, so be it.
     *   Resize seems OK with '1' and OS 10.13.
     */
    [self performSelectorOnMainThread:@selector(updateMain:) withObject:nil waitUntilDone:1];
  
  }
}

/* Route call from main loop.
 * This runs on the main thread.
 */

-(void)updateMain:(id)ignore {
  fmn_macioc.update_in_progress=1;
  fmn_macioc.framec++;
  if (intf_update(fmn_macioc.intf)<0) {
    fmn_macioc_abort("intf_update failed");
    return;
  }
  loop();
  fmn_macioc.update_in_progress=0;
}

/* Finish launching.
 * We fire the 'init' callback and launch an updater thread.
 */

-(void)applicationDidFinishLaunching:(NSNotification*)notification {
  int err=fmn_macioc_init();
  if (err<0) {
    fmn_macioc_abort("Initialization failed (%d). Aborting.",err);
  }
  [NSThread detachNewThreadSelector:@selector(mainLoop:) toTarget:self withObject:nil];
  
}

/* Termination.
 */

-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
  return NSTerminateNow;
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
  return 1;
}

-(void)applicationWillTerminate:(NSNotification*)notification {
  fmn_macioc.terminate=1;
  fmn_macioc_report_performance();
  intf_del(fmn_macioc.intf);
  fmn_macioc.intf=0;
}

/* Receive system error.
 */

-(NSError*)application:(NSApplication*)application willPresentError:(NSError*)error {
  const char *message=error.localizedDescription.UTF8String;
  fprintf(stderr,"%s\n",message);
  return error;
}

/* Change input focus.
 * intf expects this to be a video concern, but on the Mac, it's an Application thing. Defer.
 */

-(void)applicationDidBecomeActive:(NSNotification*)notification {
  if (fmn_macioc.intf) fmn_macioc_cb_focus(fmn_macioc.intf->video,1);
}

-(void)applicationDidResignActive:(NSNotification*)notification {
  if (fmn_macioc.intf) fmn_macioc_cb_focus(fmn_macioc.intf->video,0);
}

@end
