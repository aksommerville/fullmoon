#include "fmn_macwm_internal.h"

@interface FmnView : NSView {
@public
  CGDataProviderRef provider;
  CGColorSpaceRef colorspace;
  const void *fb; // from caller
  int layerInit;
}
@end

static const void *fmnview_cb_getBytePointer(void *userdata) {
  FmnView *view=userdata;
  return view->fb;
}

static size_t fmnview_cb_getBytesAtPosition(void *userdata,void *v,off_t p,size_t c) {
  FmnView *view=userdata;
  size_t limit=FMN_FBW*FMN_FBH*4;
  if (p>limit-c) c=limit-p;
  memcpy((uint8_t*)v+p,(uint8_t*)(view->fb)+p,c);
  return c;
}

static void fmnview_cb_releaseBytePointer(void *userdata,const void *fb) {
}

@implementation FmnView

-(void)dealloc {
  if (provider) CGDataProviderRelease(provider);
  if (colorspace) CGColorSpaceRelease(colorspace);
  [super dealloc];
}

-(id)init {
  [super init];
  int storage_size=FMN_FBW*FMN_FBH*4;
  
  CGDataProviderDirectCallbacks providercb={
    .getBytePointer=fmnview_cb_getBytePointer,
    .getBytesAtPosition=fmnview_cb_getBytesAtPosition,
    .releaseBytePointer=fmnview_cb_releaseBytePointer,
  };
  provider=CGDataProviderCreateDirect(self,storage_size,&providercb);
  
  colorspace=CGColorSpaceCreateDeviceRGB();

  return self;
}

-(BOOL)wantsLayer { return 1; }
-(BOOL)wantsUpdateLayer { return 1; }
-(BOOL)canDrawSubviewsIntoLayer { return 0; }

-(void)updateLayer {
  if (!fb) return;

  if (!layerInit) {
    self.layer.contentsGravity=kCAGravityResizeAspect;
    // Set the background color to black. Seriously? We have to do all this?
    CGFloat blackv[4]={0.0f,0.0f,0.0f,1.0f};
    CGColorRef black=CGColorCreate(colorspace,blackv);
    self.layer.backgroundColor=black;
    CGColorRelease(black);
    layerInit=1;
  }

  CGImageRef img=CGImageCreate(
    FMN_FBW,FMN_FBH,
    8,32,FMN_FBW*4,
    colorspace,
    kCGBitmapByteOrderDefault, // CGBitmapInfo
    provider,
    0, // decode
    0, // interpolate
    kCGRenderingIntentDefault
  );
  if (!img) return;
  
  self.layer.contents=(id)img;
  CGImageRelease(img);
}
@end

@implementation FmnWindow

/* Lifecycle.
 */

-(void)dealloc {
  if (self==fmn_macwm.window) {
    fmn_macwm.window=0;
  }
  [super dealloc];
}

-(id)initWithContentRect:(NSRect)contentRect styleMask:(NSWindowStyleMask)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag screen:(NSScreen*)screen {

  if (fmn_macwm.window) {
    return 0;
  }

  if (!(self=[super initWithContentRect:contentRect styleMask:aStyle backing:bufferingType defer:flag screen:screen])) {
    return 0;
  }
  
  fmn_macwm.window=self;
  self.delegate=self;

  [self makeKeyAndOrderFront:nil];

  return self;
}

/* Preferred public initializer.
 */

+(FmnWindow*)newWithWidth:(int)width 
  height:(int)height 
  title:(const char*)title
  fullscreen:(int)fullscreen
  intf_delegate:(const struct intf_delegate*)intf_delegate
  driver:(void*)driver
{

  NSScreen *screen=[NSScreen mainScreen];
  if (!screen) {
    return 0;
  }
  int screenw=screen.frame.size.width;
  int screenh=screen.frame.size.height;

  int margin=100;
  int scalex=(screenw-margin)/width;
  int scaley=(screenh-margin)/height;
  int scale=(scalex<scaley)?scalex:scaley;
  if (scale<1) scale=1;
  else if (scale>FMN_MACOS_MAX_INITIAL_SCALE) scale=FMN_MACOS_MAX_INITIAL_SCALE;
  width*=scale;
  height*=scale;

  if (width>screenw) width=screenw;
  if (width<0) width=0;
  if (height>screenh) height=screenh;
  if (height<0) height=0;

  NSRect bounds=NSMakeRect((screenw>>1)-(width>>1),(screenh>>1)-(height>>1),width,height);

  NSWindowStyleMask styleMask=
    NSWindowStyleMaskTitled|
    NSWindowStyleMaskClosable|
    NSWindowStyleMaskMiniaturizable|
    NSWindowStyleMaskResizable|
  0;
  
  FmnWindow *window=[[FmnWindow alloc]
    initWithContentRect:bounds
    styleMask:styleMask
    backing:NSBackingStoreBuffered
    defer:0
    screen:0
  ];
  if (!window) return 0;
  
  if (intf_delegate) memcpy(&window->intf_delegate,intf_delegate,sizeof(struct intf_delegate));
  window->driver=driver;

  window->w=width;
  window->h=height;
  if (driver) {
    ((struct video_driver*)driver)->w=width;
    ((struct video_driver*)driver)->h=height;
  }
  
  window.releasedWhenClosed=0;
  window.acceptsMouseMovedEvents=TRUE;
  window->cursor_visible=1;
  fmn_macwm_show_cursor(0);

  if (title) {
    window.title=[NSString stringWithUTF8String:title];
  }

  if (!(window.contentView=[FmnView new])) {
    [window release];
    return 0;
  }

  if (fullscreen) {
    [window toggleFullScreen:window];
  }
  
  return window;
}

/* Receive keyboard events.
 */

-(void)keyDown:(NSEvent*)event {

  if (event.modifierFlags&NSEventModifierFlagCommand) {
    return;
  }

  int codepoint=0;
  const char *src=event.characters.UTF8String;
  if (src&&src[0]) {
    if (fmn_macwm_decode_utf8(&codepoint,src,-1)<1) codepoint=0;
    else codepoint=fmn_macwm_translate_codepoint(codepoint);
  }

  int state=event.ARepeat?2:1;
  int key=fmn_macwm_translate_keysym(event.keyCode);
  if (!codepoint&&!key) return;

  fmn_macwm_record_key_down(event.keyCode);

  if (key&&intf_delegate.key) {
    if (intf_delegate.key(driver,key,state)<0) {
      fmn_macwm_abort("Error handling keyDown");
    }
  }
  if (codepoint&&intf_delegate.text) {
    if (intf_delegate.text(driver,codepoint)<0) {
      fmn_macwm_abort("Error handling keyDown");
    }
  }
}

-(void)keyUp:(NSEvent*)event {
  fmn_macwm_release_key_down(event.keyCode);
  int key=fmn_macwm_translate_keysym(event.keyCode);
  if (!key) return;
  if (intf_delegate.key) {
    if (intf_delegate.key(driver,key,0)<0) {
      fmn_macwm_abort("Error handling keyUp");
    }
  }
}

-(void)flagsChanged:(NSEvent*)event {
  int nmodifiers=(int)event.modifierFlags;
  if (nmodifiers!=modifiers) {
    int omodifiers=modifiers;
    modifiers=nmodifiers;
    int mask=1; for (;mask;mask<<=1) {
    
      if ((nmodifiers&mask)&&!(omodifiers&mask)) {
        int key=fmn_macwm_translate_modifier(mask);
        if (key) {
          if (intf_delegate.key) {
            if (intf_delegate.key(driver,key,1)<0) {
              fmn_macwm_abort("Error handling keyDown");
            }
          }
        }

      } else if (!(nmodifiers&mask)&&(omodifiers&mask)) {
        int key=fmn_macwm_translate_modifier(mask);
        if (key) {
          if (intf_delegate.key) {
            if (intf_delegate.key(driver,key,0)<0) {
              fmn_macwm_abort("Error handling keyUp");
            }
          }
        }
      }
    }
  }
}

/* Mouse events.
 */

static void fmn_macwm_event_mouse_motion(NSPoint loc) {

  int wasin=fmn_macwm_cursor_within_window();
  fmn_macwm.window->mousex=loc.x;
  fmn_macwm.window->mousey=fmn_macwm.window->h-loc.y;
  int nowin=fmn_macwm_cursor_within_window();

  if (!fmn_macwm.window->cursor_visible) {
    if (wasin&&!nowin) {
      [NSCursor unhide];
    } else if (!wasin&&nowin) {
      [NSCursor hide];
    }
  }

  const struct intf_delegate *dl=&fmn_macwm.window->intf_delegate;
  if (dl->mmotion) {
    if (dl->mmotion(fmn_macwm.window->driver,fmn_macwm.window->mousex,fmn_macwm.window->mousey)<0) {
      fmn_macwm_abort("Error handling mouse motion");
    }
  }
}

-(void)mouseMoved:(NSEvent*)event { fmn_macwm_event_mouse_motion(event.locationInWindow); }
-(void)mouseDragged:(NSEvent*)event { fmn_macwm_event_mouse_motion(event.locationInWindow); }
-(void)rightMouseDragged:(NSEvent*)event { fmn_macwm_event_mouse_motion(event.locationInWindow); }
-(void)otherMouseDragged:(NSEvent*)event { fmn_macwm_event_mouse_motion(event.locationInWindow); }

-(void)scrollWheel:(NSEvent*)event {
  int dx=-event.deltaX;
  int dy=-event.deltaY;
  if (!dx&&!dy) return;
  if (intf_delegate.mwheel) {
    if (intf_delegate.mwheel(driver,dx,dy)<0) {
      fmn_macwm_abort("Error handling mouse wheel");
    }
  }
}

static void fmn_macwm_event_mouse_button(int btnid,int value) {

  /* When you click in the window's title bar, MacOS sends mouseDown but not mouseUp at the end of it.
   * That's fucked up and it fucks us right up.
   * So we ignore mouseDown when the pointer is outside our bounds -- that's sensible anyway.
   */
  if (value) {
    if ((fmn_macwm.window->mousex<0)||(fmn_macwm.window->mousex>=fmn_macwm.window->w)) return;
    if ((fmn_macwm.window->mousey<0)||(fmn_macwm.window->mousey>=fmn_macwm.window->h)) return;
  }

  if (!(btnid=fmn_macwm_translate_mbtn(btnid))) return;
  if (fmn_macwm.window->intf_delegate.mbutton) {
    if (fmn_macwm.window->intf_delegate.mbutton(fmn_macwm.window->driver,btnid,value)<0) {
      fmn_macwm_abort("Error handling mouse button");
    }
  }
}

-(void)mouseDown:(NSEvent*)event { fmn_macwm_event_mouse_button(1,1); }
-(void)mouseUp:(NSEvent*)event { fmn_macwm_event_mouse_button(1,0); }
-(void)rightMouseDown:(NSEvent*)event { fmn_macwm_event_mouse_button(2,1); }
-(void)rightMouseUp:(NSEvent*)event { fmn_macwm_event_mouse_button(2,0); }
-(void)otherMouseDown:(NSEvent*)event { fmn_macwm_event_mouse_button(event.buttonNumber,1); }
-(void)otherMouseUp:(NSEvent*)event { fmn_macwm_event_mouse_button(event.buttonNumber,0); }

/* Resize events.
 */

-(void)reportResize:(NSSize)size {

  /* Don't report redundant events (they are sent) */
  int nw=size.width;
  int nh=size.height;
  if ((w==nw)&&(h==nh)) return;
  w=nw;
  h=nh;
  if (driver) {
    driver->w=w;
    driver->h=h;
  }
  if (intf_delegate.resize) {
    if (intf_delegate.resize(driver,w,h)<0) {
      fmn_macwm_abort("Error handling resize");
    }
  }
}

-(NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize {
  [self reportResize:[self contentRectForFrameRect:(NSRect){.size=frameSize}].size];
  return frameSize;
}

-(NSSize)window:(NSWindow*)window willUseFullScreenContentSize:(NSSize)proposedSize {
  [self reportResize:proposedSize];
  return proposedSize;
}

-(void)windowWillStartLiveResize:(NSNotification*)note {
}

-(void)windowDidEndLiveResize:(NSNotification*)note {
}

-(void)windowDidEnterFullScreen:(NSNotification*)note {
  FmnWindow *window=(FmnWindow*)note.object;
  if ([window isKindOfClass:FmnWindow.class]) {
    window->fullscreen=1;
  }
  if (fmn_macwm_drop_all_keys()<0) {
    fmn_macwm_abort("Failure in key release handler.");
  }
}

-(void)windowDidExitFullScreen:(NSNotification*)note {
  FmnWindow *window=(FmnWindow*)note.object;
  if ([window isKindOfClass:FmnWindow.class]) {
    window->fullscreen=0;
  }
  if (fmn_macwm_drop_all_keys()<0) {
    fmn_macwm_abort("Failure in key release handler.");
  }
}

/* NSWindowDelegate hooks.
 */

-(BOOL)window:(NSWindow*)window shouldDragDocumentWithEvent:(NSEvent*)event from:(NSPoint)dragImageLocation withPasteboard:(NSPasteboard*)pasteboard {
  return 1;
}

@end

void fmn_macwm_replace_fb(const void *src) {
  if (!fmn_macwm.window) return;
  FmnView *view=fmn_macwm.window.contentView;
  view->fb=src;
  view.needsDisplay=1;
}
