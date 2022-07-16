#ifndef FMN_X11_INTERNAL_H
#define FMN_X11_INTERNAL_H

#include "opt/intf/intf.h"
#include "game/fullmoon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

// Required only for making intelligent initial-size decisions in a multi-monitor setting.
// apt install libxinerama-dev
// Or neuter fmn_x11_driver.c:fmn_x11_estimate_monitor_size(), if you don't want it.
#include <X11/extensions/Xinerama.h>

#define KeyRepeat (LASTEvent+2)
#define FMN_X11_KEY_REPEAT_INTERVAL 10

#define FMN_X11_SCALE_LIMIT 8
#define FMN_X11_ICON_SIZE_LIMIT 128

struct video_driver_x11 {
  struct video_driver hdr;
  
  Display *dpy;
  int screen;
  Window win;
  GC gc;
  
  // If (dstdirty), we need to recalculate image size and position.
  // That could mean destroying and rebuilding the image object.
  // This image is sized to the logical output: fb<=image<=win
  XImage *image;
  int dstx,dsty;
  int dstdirty;
  int rshift,gshift,bshift;
  int scale;
  
  Atom atom_WM_PROTOCOLS;
  Atom atom_WM_DELETE_WINDOW;
  Atom atom__NET_WM_STATE;
  Atom atom__NET_WM_STATE_FULLSCREEN;
  Atom atom__NET_WM_STATE_ADD;
  Atom atom__NET_WM_STATE_REMOVE;
  Atom atom__NET_WM_ICON;
  Atom atom__NET_WM_ICON_NAME;
  Atom atom__NET_WM_NAME;
  Atom atom_WM_CLASS;
  Atom atom_STRING;
  Atom atom_UTF8_STRING;
  
  int screensaver_suppressed;
  int focus;
};

#define DRIVER ((struct video_driver_x11*)driver)

int _x11_update(struct video_driver *driver);

int fmn_x11_codepoint_from_keysym(int keysym);
int fmn_x11_usb_usage_from_keysym(int keysym);

void fmn_x11_scale(struct video_driver *driver,const void *src);

#endif
