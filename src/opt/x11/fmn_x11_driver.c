#include "fmn_x11_internal.h"

/* Cleanup.
 */
 
static void _x11_del(struct video_driver *driver) {
  if (DRIVER->dpy) {
    if (DRIVER->image) XDestroyImage(DRIVER->image);
    if (DRIVER->gc) XFreeGC(DRIVER->dpy,DRIVER->gc);
    XCloseDisplay(DRIVER->dpy);
  }
}

/* Set window icon.
 */
 
static void fmn_x11_copy_pixels(long *dst,const uint8_t *src,int c) {
  for (;c-->0;dst++,src+=4) {
    #if BYTE_ORDER==BIG_ENDIAN
      /* https://standards.freedesktop.org/wm-spec/wm-spec-1.3.html
       * """
       * This is an array of 32bit packed CARDINAL ARGB with high byte being A, low byte being B.
       * The first two cardinals are width, height. Data is in rows, left to right and top to bottom.
       * """
       * I take this to mean that big-endian should work the same as little-endian.
       * But I'm nervous about it because:
       *  - I don't have any big-endian machines handy for testing.
       *  - The data type is "long" which is not always "32bit" as they say. (eg it is 64 on my box)
       */
      *dst=(src[3]<<24)|(src[0]<<16)|(src[1]<<8)|src[2];
    #else
      *dst=(src[3]<<24)|(src[0]<<16)|(src[1]<<8)|src[2];
    #endif
  }
}
 
static int fmn_x11_set_icon(struct video_driver *driver,const void *rgba,int w,int h) {
  if (!rgba||(w<1)||(h<1)||(w>FMN_X11_ICON_SIZE_LIMIT)||(h>FMN_X11_ICON_SIZE_LIMIT)) {
    // Bad request, whatever.
    return 0;
  }
  int length=2+w*h;
  long *pixels=malloc(sizeof(long)*length);
  if (!pixels) return -1;
  pixels[0]=w;
  pixels[1]=h;
  fmn_x11_copy_pixels(pixels+2,rgba,w*h);
  XChangeProperty(DRIVER->dpy,DRIVER->win,DRIVER->atom__NET_WM_ICON,XA_CARDINAL,32,PropModeReplace,(unsigned char*)pixels,length);
  free(pixels);
  return 0;
}

/* Get the size of the monitor we're going to display on.
 * NOT the size of the logical desktop.
 * We don't actually know which monitor will be chosen, and we don't want to force it, so use the smallest.
 */
 
static void fmn_x11_estimate_monitor_size(int *w,int *h,struct video_driver *driver) {
  *w=*h=0;
  
  // First, ask Xinerama.
  int infoc=0;
  XineramaScreenInfo *infov=XineramaQueryScreens(DRIVER->dpy,&infoc);
  if (infov) {
    if (infoc>0) {
      *w=infov[0].width;
      *h=infov[0].height;
      int i=infoc; while (i-->1) {
        if ((infov[i].width<*w)||(infov[i].height<*h)) {
          *w=infov[i].width;
          *h=infov[i].height;
        }
      }
    }
    XFree(infov);
  }
  
  // ...if we haven't got anything valid, use the logical desktop size.
  if ((*w<1)||(*h<1)) {
    *w=DisplayWidth(DRIVER->dpy,0);
    *h=DisplayHeight(DRIVER->dpy,0);
    // ...and still nothing? Fuck it.
    if ((*w<1)||(*h<1)) {
      *w=640;
      *h=480;
    }
  }
}

/* Init.
 */
 
static int _x11_init(struct video_driver *driver) {

  DRIVER->focus=1;
  DRIVER->dstdirty=1;
  
  if (!(DRIVER->dpy=XOpenDisplay(0))) return -1;
  DRIVER->screen=DefaultScreen(DRIVER->dpy);

  #define GETATOM(tag) DRIVER->atom_##tag=XInternAtom(DRIVER->dpy,#tag,0);
  GETATOM(WM_PROTOCOLS)
  GETATOM(WM_DELETE_WINDOW)
  GETATOM(_NET_WM_STATE)
  GETATOM(_NET_WM_STATE_FULLSCREEN)
  GETATOM(_NET_WM_STATE_ADD)
  GETATOM(_NET_WM_STATE_REMOVE)
  GETATOM(_NET_WM_ICON)
  GETATOM(_NET_WM_ICON_NAME)
  GETATOM(_NET_WM_NAME)
  GETATOM(STRING)
  GETATOM(UTF8_STRING)
  GETATOM(WM_CLASS)
  #undef GETATOM
  
  // Initial scale. (driver->w,driver->h) is initialized to the framebuffer size, which we should expect to be way smaller than the screen.
  // Reduce the assessed monitor's size to 3/4, to allow for WM chrome, and to not be so in-your-face about it.
  {
    int monw,monh;
    fmn_x11_estimate_monitor_size(&monw,&monh,driver);
    monw=(monw*3)/4;
    monh=(monh*3)/4;
    int xscale=monw/driver->w;
    int yscale=monh/driver->h;
    DRIVER->scale=(xscale<yscale)?xscale:yscale;
    if (DRIVER->scale<1) DRIVER->scale=1;
    else if (DRIVER->scale>FMN_X11_SCALE_LIMIT) DRIVER->scale=FMN_X11_SCALE_LIMIT;
    int fbw=driver->w,fbh=driver->h;
    driver->w*=DRIVER->scale;
    driver->h*=DRIVER->scale;
  }
  
  XSetWindowAttributes wattr={
    .background_pixel=0,
    .event_mask=
      StructureNotifyMask|
      KeyPressMask|KeyReleaseMask|
      FocusChangeMask|
      //TODO Leaving off mouse events, though in all other ways we are prepared to deliver them. Decide whether they're useful.
    0,
  };
  
  if (!(DRIVER->win=XCreateWindow(
    DRIVER->dpy,RootWindow(DRIVER->dpy,DRIVER->screen),
    0,0,driver->w,driver->h,0,
    DefaultDepth(DRIVER->dpy,DRIVER->screen),InputOutput,CopyFromParent,
    CWBackPixel|CWBorderPixel|CWColormap|CWEventMask,&wattr
  ))) return -1;
  
  if (driver->fullscreen) {
    XChangeProperty(
      DRIVER->dpy,DRIVER->win,
      DRIVER->atom__NET_WM_STATE,
      XA_ATOM,32,PropModeReplace,
      (unsigned char*)&DRIVER->atom__NET_WM_STATE_FULLSCREEN,1
    );
    driver->fullscreen=1;
  }
  
  if (driver->delegate.title) {
  
    // I've seen these properties in GNOME 2, unclear whether they might still be in play:
    XTextProperty prop={.value=(void*)driver->delegate.title,.encoding=DRIVER->atom_STRING,.format=8,.nitems=0};
    while (prop.value[prop.nitems]) prop.nitems++;
    XSetWMName(DRIVER->dpy,DRIVER->win,&prop);
    XSetWMIconName(DRIVER->dpy,DRIVER->win,&prop);
    XSetTextProperty(DRIVER->dpy,DRIVER->win,&prop,DRIVER->atom__NET_WM_ICON_NAME);
    
    // This one becomes the window title and bottom-bar label, in GNOME 3:
    prop.encoding=DRIVER->atom_UTF8_STRING;
    XSetTextProperty(DRIVER->dpy,DRIVER->win,&prop,DRIVER->atom__NET_WM_NAME);
    
    // This daffy bullshit becomes the Alt-Tab text in GNOME 3:
    {
      char tmp[256];
      int len=prop.nitems+1+prop.nitems;
      if (len<sizeof(tmp)) {
        memcpy(tmp,prop.value,prop.nitems);
        tmp[prop.nitems]=0;
        memcpy(tmp+prop.nitems+1,prop.value,prop.nitems);
        tmp[prop.nitems+1+prop.nitems]=0;
        prop.value=tmp;
        prop.nitems=prop.nitems+1+prop.nitems;
        prop.encoding=DRIVER->atom_STRING;
        XSetTextProperty(DRIVER->dpy,DRIVER->win,&prop,DRIVER->atom_WM_CLASS);
      }
    }
  }
  
  fmn_x11_set_icon(driver,driver->delegate.icon_rgba,driver->delegate.iconw,driver->delegate.iconh);
  
  XMapWindow(DRIVER->dpy,DRIVER->win);
  
  XSync(DRIVER->dpy,0);
  
  XSetWMProtocols(DRIVER->dpy,DRIVER->win,&DRIVER->atom_WM_DELETE_WINDOW,1);
  
  // Hide cursor.
  XColor color;
  Pixmap pixmap=XCreateBitmapFromData(DRIVER->dpy,DRIVER->win,"\0\0\0\0\0\0\0\0",1,1);
  Cursor cursor=XCreatePixmapCursor(DRIVER->dpy,pixmap,pixmap,&color,&color,0,0);
  XDefineCursor(DRIVER->dpy,DRIVER->win,cursor);
  XFreeCursor(DRIVER->dpy,cursor);
  XFreePixmap(DRIVER->dpy,pixmap);
  
  if (!(DRIVER->gc=XCreateGC(DRIVER->dpy,DRIVER->win,0,0))) return -1;
  
  return 0;
}

/* Select framebuffer's output bounds.
 */
 
static int fmn_x11_recalculate_output_bounds(struct video_driver *driver) {

  /* First decide the scale factor:
   *  - At least 1.
   *  - At most FMN_X11_SCALE_LIMIT.
   *  - Or lesser of winw/fbw,winh/fbh.
   */
  int scalex=driver->w/driver->delegate.fbw;
  int scaley=driver->h/driver->delegate.fbh;
  DRIVER->scale=(scalex<scaley)?scalex:scaley;
  if (DRIVER->scale<1) DRIVER->scale=1;
  else if (DRIVER->scale>FMN_X11_SCALE_LIMIT) DRIVER->scale=FMN_X11_SCALE_LIMIT;
  
  /* From there, size and position are trivial:
   */
  int dstw=driver->delegate.fbw*DRIVER->scale;
  int dsth=driver->delegate.fbh*DRIVER->scale;
  DRIVER->dstx=(driver->w>>1)-(dstw>>1);
  DRIVER->dsty=(driver->h>>1)-(dsth>>1);
  
  /* If the image is not yet created, or doesn't match the calculated size, rebuild it.
   */
  if (!DRIVER->image||(DRIVER->image->width!=dstw)||(DRIVER->image->height!=dsth)) {
    if (DRIVER->image) {
      XDestroyImage(DRIVER->image);
      DRIVER->image=0;
    }
    void *pixels=malloc(dstw*4*dsth);
    if (!pixels) return -1;
    if (!(DRIVER->image=XCreateImage(
      DRIVER->dpy,DefaultVisual(DRIVER->dpy,DRIVER->screen),24,ZPixmap,0,pixels,dstw,dsth,32,dstw*4
    ))) {
      free(pixels);
      return -1;
    }
    
    // And recalculate channel shifts...
    if (!DRIVER->image->red_mask||!DRIVER->image->green_mask||!DRIVER->image->blue_mask) return -1;
    uint32_t m;
    DRIVER->rshift=0; m=DRIVER->image->red_mask;   for (;!(m&1);m>>=1,DRIVER->rshift++) ; if (m!=0xff) return -1;
    DRIVER->gshift=0; m=DRIVER->image->green_mask; for (;!(m&1);m>>=1,DRIVER->gshift++) ; if (m!=0xff) return -1;
    DRIVER->bshift=0; m=DRIVER->image->blue_mask;  for (;!(m&1);m>>=1,DRIVER->bshift++) ; if (m!=0xff) return -1;
  }
  
  return 0;
}

/* Swap buffers.
 */
 
static int _x11_swap(struct video_driver *driver,const void *fb) {

  // Recalculate geometry, and maybe reallocate scale-up buffer, if the window size changed.
  if (DRIVER->dstdirty) {
    if (fmn_x11_recalculate_output_bounds(driver)<0) return -1;
    DRIVER->dstdirty=0;
    XClearWindow(DRIVER->dpy,DRIVER->win);
  }
  
  fmn_x11_scale(driver,fb);
  
  XPutImage(DRIVER->dpy,DRIVER->win,DRIVER->gc,DRIVER->image,0,0,DRIVER->dstx,DRIVER->dsty,DRIVER->image->width,DRIVER->image->height);
  
  DRIVER->screensaver_suppressed=0;
  return 0;
}

/* Fullscreen.
 */

static void fmn_x11_set_fullscreen(struct video_driver *driver,int state) {
  XEvent evt={
    .xclient={
      .type=ClientMessage,
      .message_type=DRIVER->atom__NET_WM_STATE,
      .send_event=1,
      .format=32,
      .window=DRIVER->win,
      .data={.l={
        state,
        DRIVER->atom__NET_WM_STATE_FULLSCREEN,
      }},
    }
  };
  XSendEvent(DRIVER->dpy,RootWindow(DRIVER->dpy,DRIVER->screen),0,SubstructureNotifyMask|SubstructureRedirectMask,&evt);
  XFlush(DRIVER->dpy);
  driver->fullscreen=state;
}
 
static int _x11_fullscreen(struct video_driver *driver,int state) {
  if (state>0) {
    if (driver->fullscreen) return 1;
    fmn_x11_set_fullscreen(driver,1);
  } else if (state==0) {
    if (!driver->fullscreen) return 0;
    fmn_x11_set_fullscreen(driver,0);
  }
  return driver->fullscreen;
}

/* Screensaver.
 */
 
static void _x11_suppress_screensaver(struct video_driver *driver) {
  if (!DRIVER->screensaver_suppressed) return;
  XForceScreenSaver(DRIVER->dpy,ScreenSaverReset);
  DRIVER->screensaver_suppressed=1;
}

/* Type definition.
 */
 
const struct video_driver_type video_driver_type_x11={
  .name="x11",
  .desc="X11, for desktop Linux.",
  .objlen=sizeof(struct video_driver_x11),
  .del=_x11_del,
  .init=_x11_init,
  .update=_x11_update,
  .swap=_x11_swap,
  .fullscreen=_x11_fullscreen,
  .suppress_screensaver=_x11_suppress_screensaver,
};
