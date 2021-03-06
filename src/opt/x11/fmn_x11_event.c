#include "fmn_x11_internal.h"

/* Key press, release, or repeat.
 */
 
static int fmn_x11_evt_key(struct video_driver *driver,XKeyEvent *evt,int value) {

  /* Pass the raw keystroke. */
  if (driver->delegate.key) {
    KeySym keysym=XkbKeycodeToKeysym(DRIVER->dpy,evt->keycode,0,0);
    if (keysym) {
      int keycode=fmn_x11_usb_usage_from_keysym((int)keysym);
      if (keycode) {
        if (driver->delegate.key(driver,keycode,value)<0) return -1;
      }
    }
  }
  
  /* Pass text if press or repeat, and text can be acquired. */
  if (driver->delegate.text) {
    int shift=(evt->state&ShiftMask)?1:0;
    KeySym tkeysym=XkbKeycodeToKeysym(DRIVER->dpy,evt->keycode,0,shift);
    if (shift&&!tkeysym) { // If pressing shift makes this key "not a key anymore", fuck that and pretend shift is off
      tkeysym=XkbKeycodeToKeysym(DRIVER->dpy,evt->keycode,0,0);
    }
    if (tkeysym) {
      int codepoint=fmn_x11_codepoint_from_keysym(tkeysym);
      if (codepoint && (evt->type == KeyPress || evt->type == KeyRepeat)) {
        if (driver->delegate.text(driver,codepoint)<0) return -1;
      }
    }
  }
  
  return 0;
}

/* Mouse events.
 */
 
static int fmn_x11_evt_mbtn(struct video_driver *driver,XButtonEvent *evt,int value) {
  switch (evt->button) {
    case 1: if (driver->delegate.mbutton) return driver->delegate.mbutton(driver,1,value); break;
    case 2: if (driver->delegate.mbutton) return driver->delegate.mbutton(driver,3,value); break;
    case 3: if (driver->delegate.mbutton) return driver->delegate.mbutton(driver,2,value); break;
    case 4: if (value&&driver->delegate.mwheel) return driver->delegate.mwheel(driver,0,-1); break;
    case 5: if (value&&driver->delegate.mwheel) return driver->delegate.mwheel(driver,0,1); break;
    case 6: if (value&&driver->delegate.mwheel) return driver->delegate.mwheel(driver,-1,0); break;
    case 7: if (value&&driver->delegate.mwheel) return driver->delegate.mwheel(driver,1,0); break;
  }
  return 0;
}

static int fmn_x11_evt_mmotion(struct video_driver *driver,XMotionEvent *evt) {
  if (driver->delegate.mmotion) {
    if (driver->delegate.mmotion(driver,evt->x,evt->y)<0) return -1;
  }
  return 0;
}

/* Client message.
 */
 
static int fmn_x11_evt_client(struct video_driver *driver,XClientMessageEvent *evt) {
  if (evt->message_type==DRIVER->atom_WM_PROTOCOLS) {
    if (evt->format==32) {
      if (evt->data.l[0]==DRIVER->atom_WM_DELETE_WINDOW) {
        if (driver->delegate.close) {
          return driver->delegate.close(driver);
        }
      }
    }
  }
  return 0;
}

/* Configuration event (eg resize).
 */
 
static int fmn_x11_evt_configure(struct video_driver *driver,XConfigureEvent *evt) {
  int nw=evt->width,nh=evt->height;
  if ((nw!=driver->w)||(nh!=driver->h)) {
    driver->w=nw;
    driver->h=nh;
    DRIVER->dstdirty=1;
    if (driver->delegate.resize) {
      if (driver->delegate.resize(driver,nw,nh)<0) {
        return -1;
      }
    }
  }
  return 0;
}

/* Focus.
 */
 
static int fmn_x11_evt_focus(struct video_driver *driver,XFocusInEvent *evt,int value) {
  if (value==DRIVER->focus) return 0;
  DRIVER->focus=value;
  if (driver->delegate.focus) {
    return driver->delegate.focus(driver,value);
  }
  return 0;
}

/* Process one event.
 */
 
static int fmn_x11_receive_event(struct video_driver *driver,XEvent *evt) {
  if (!evt) return -1;
  switch (evt->type) {
  
    case KeyPress: return fmn_x11_evt_key(driver,&evt->xkey,1);
    case KeyRelease: return fmn_x11_evt_key(driver,&evt->xkey,0);
    case KeyRepeat: return fmn_x11_evt_key(driver,&evt->xkey,2);
    
    case ButtonPress: return fmn_x11_evt_mbtn(driver,&evt->xbutton,1);
    case ButtonRelease: return fmn_x11_evt_mbtn(driver,&evt->xbutton,0);
    case MotionNotify: return fmn_x11_evt_mmotion(driver,&evt->xmotion);
    
    case ClientMessage: return fmn_x11_evt_client(driver,&evt->xclient);
    
    case ConfigureNotify: return fmn_x11_evt_configure(driver,&evt->xconfigure);
    
    case FocusIn: return fmn_x11_evt_focus(driver,&evt->xfocus,1);
    case FocusOut: return fmn_x11_evt_focus(driver,&evt->xfocus,0);
    
  }
  return 0;
}

/* Update.
 */
 
int _x11_update(struct video_driver *driver) {
  int evtc=XEventsQueued(DRIVER->dpy,QueuedAfterFlush);
  while (evtc-->0) {
    XEvent evt={0};
    XNextEvent(DRIVER->dpy,&evt);
    
    /* If we detect an auto-repeated key, drop one of the events, and turn the other into KeyRepeat.
     * This is a hack to force single events for key repeat.
     */
    if ((evtc>0)&&(evt.type==KeyRelease)) {
      XEvent next={0};
      XNextEvent(DRIVER->dpy,&next);
      evtc--;
      if ((next.type==KeyPress)&&(evt.xkey.keycode==next.xkey.keycode)&&(evt.xkey.time>=next.xkey.time-FMN_X11_KEY_REPEAT_INTERVAL)) {
        evt.type=KeyRepeat;
        if (fmn_x11_receive_event(driver,&evt)<0) return -1;
      } else {
        if (fmn_x11_receive_event(driver,&evt)<0) return -1;
        if (fmn_x11_receive_event(driver,&next)<0) return -1;
      }
    } else {
      if (fmn_x11_receive_event(driver,&evt)<0) return -1;
    }
  }
  return 1;
}
