#ifndef FMN_MACWM_INTERNAL_H
#define FMN_MACWM_INTERNAL_H

// Link: -framework Cocoa -framework OpenGL

#include "game/fullmoon.h"
#include "opt/intf/intf.h"
#include <OpenGL/gl.h>

#ifdef __OBJC__
#include <Cocoa/Cocoa.h>

/* Declaration of window class.
 */

@interface FmnWindow : NSWindow <NSWindowDelegate> {
@public
  NSOpenGLContext *context;
  int cursor_visible;
  int w,h;
  int mousex,mousey;
  int modifiers;
  int fullscreen;
  struct intf_delegate intf_delegate;
  struct video_driver *driver;//WEAK, just so we can give it to delegate calls
}

/* Provide the framebuffer size here.
 * We will scale up to a reasonable size considering the main screen's size.
 */
+(FmnWindow*)newWithWidth:(int)width 
  height:(int)height 
  title:(const char*)title
  fullscreen:(int)fullscreen
  intf_delegate:(const struct intf_delegate*)intf_delegate
  driver:(void*)driver
;

-(int)beginFrame;
-(int)endFrame;

@end
#else
typedef void FmnWindow;
#endif

/* Globals.
 */

#define FMN_MACWM_KEYS_DOWN_LIMIT 8

extern struct fmn_macwm {
  FmnWindow *window;
  int keys_down[FMN_MACWM_KEYS_DOWN_LIMIT];
} fmn_macwm;

/* Miscellaneous.
 */

void fmn_macwm_abort(const char *fmt,...);

int fmn_macwm_decode_utf8(int *dst,const void *src,int srcc);
int fmn_macwm_translate_codepoint(int src);
int fmn_macwm_translate_keysym(int src);
int fmn_macwm_translate_modifier(int src);
int fmn_macwm_translate_mbtn(int src);

int fmn_macwm_record_key_down(int key);
int fmn_macwm_release_key_down(int key);
int fmn_macwm_drop_all_keys();

int fmn_macwm_cursor_within_window();

/* Old public API, no longer exposed publicly.
 * We could get all these init params off the delegate ourselves, I'm just trying to keep the 'intf' connection as loose as possible.
 */
int fmn_macwm_init(
  int w,int h,
  int fullscreen,
  const char *title,
  const struct intf_delegate *intf_delegate,
  void *driver
);
void fmn_macwm_quit();
int fmn_macwm_show_cursor(int show);
int fmn_macwm_toggle_fullscreen();
int fmn_macwm_begin_video();
int fmn_macwm_end_video();

#endif
