#include "game/fullmoon.h"
#include "game/fmn_data.h"
#include "game/fmn_title.h"
#include "game/fmn_play.h"
#include "game/fmn_pause.h"
#include "game/fmn_password.h"
#include <string.h>

/* Globals.
 */

static uint8_t fb_storage[FMN_FB_SIZE_BYTES]={0};
static struct fmn_image fb={
  .v=fb_storage,
  .w=FMN_FBW,
  .h=FMN_FBH,
  .stride=FMN_FB_STRIDE,
  .fmt=FMN_FBFMT,
  .writeable=1,
};
static uint16_t pvinput=0;

static uint8_t uimode=0;

/* Change UI mode (or noop if already there).
 */
 
void fmn_set_uimode(uint8_t mode) {
  if (mode==uimode) return;
  switch (uimode) {
    case FMN_UIMODE_TITLE: fmn_title_end(); break;
    case FMN_UIMODE_PLAY: fmn_play_end(); break;
    case FMN_UIMODE_PAUSE: fmn_pause_end(); break;
    case FMN_UIMODE_PASSWORD: fmn_password_end(); break;
  }
  switch (uimode=mode) {
    case FMN_UIMODE_TITLE: fmn_title_begin(); break;
    case FMN_UIMODE_PLAY: fmn_play_begin(); break;
    case FMN_UIMODE_PAUSE: fmn_pause_begin(); break;
    case FMN_UIMODE_PASSWORD: fmn_password_begin(); break;
  }
}

/* Setup.
 */

void setup() {
  fmn_platform_init();
  fmn_set_uimode(FMN_UIMODE_TITLE);
}

/* Update.
 */
 
void loop() {
  fmn_platform_update();
  
  uint16_t input=fmn_platform_read_input();
  if (input!=pvinput) {
    switch (uimode) {
      case FMN_UIMODE_TITLE: fmn_title_input(input,pvinput); break;
      case FMN_UIMODE_PLAY: fmn_play_input(input,pvinput); break;
      case FMN_UIMODE_PAUSE: fmn_pause_input(input,pvinput); break;
      case FMN_UIMODE_PASSWORD: fmn_password_input(input,pvinput); break;
    }
    pvinput=input;
  }
  
  switch (uimode) {
    case FMN_UIMODE_TITLE: fmn_title_update(); fmn_title_render(&fb); break;
    case FMN_UIMODE_PLAY: fmn_play_update(); fmn_play_render(&fb); break;
    case FMN_UIMODE_PAUSE: fmn_pause_update(); fmn_pause_render(&fb); break;
    case FMN_UIMODE_PASSWORD: fmn_password_update(); fmn_password_render(&fb); break;
  }
  
  fmn_platform_send_framebuffer(fb.v);
}
