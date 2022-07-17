#include "fullmoon.h"
#include "fmn_play.h"
#include "fmn_data.h"
#include "game/model/fmn_hero.h"
#include "game/model/fmn_map.h"
#include <string.h>

/* Globals.
 */
 
static uint8_t bgbits_storage[(FMN_FBW*FMN_FBH)>>3];
static struct fmn_image bgbits={
  .v=bgbits_storage,
  .w=FMN_FBW,
  .h=FMN_FBH,
  .stride=FMN_FBW,
  .fmt=FMN_FBFMT,
  .writeable=1,
};
static uint8_t bgbitsdirty=1;
 
/* Begin.
 * Mind that this only means "ui mode changed"; not necessarily "start a fresh game".
 * There will always be fmn_game_reset() or fmn_game_reset_with_password() before the first fmn_play_begin().
 */
 
void fmn_play_begin() {
}

/* End.
 * Could be switching out to the pause menu; not necessarily "end of game".
 */

void fmn_play_end() {
}

/* Input.
 */
 
void fmn_play_input(uint16_t input,uint16_t prev) {

  if ((input&FMN_BUTTON_B)&&!(prev&FMN_BUTTON_B)) {
    fmn_set_uimode(FMN_UIMODE_PAUSE);
  }
  
  int8_t dx=0,dy=0;
  switch (input&(FMN_BUTTON_LEFT|FMN_BUTTON_RIGHT)) {
    case FMN_BUTTON_LEFT: dx=-1; break;
    case FMN_BUTTON_RIGHT: dx=1; break;
  }
  switch (input&(FMN_BUTTON_UP|FMN_BUTTON_DOWN)) {
    case FMN_BUTTON_UP: dy=-1; break;
    case FMN_BUTTON_DOWN: dy=1; break;
  }
  fmn_hero_set_input(dx,dy,input&FMN_BUTTON_A);

  //XXX TEMP stateless dpad to navigate screenfuls
  #define PRESS(tag) ((input&FMN_BUTTON_##tag)&&!(prev&FMN_BUTTON_##tag))
  if (PRESS(LEFT)) fmn_game_navigate(-1,0);
  if (PRESS(RIGHT)) fmn_game_navigate(1,0);
  if (PRESS(UP)) fmn_game_navigate(0,-1);
  if (PRESS(DOWN)) fmn_game_navigate(0,1);
}

/* Update.
 */
 
void fmn_play_update() {
}

/* Render bgbits.
 */
 
void fmn_bgbits_dirty() {
  bgbitsdirty=1;
}
 
static void fmn_render_bgbits() {
  uint8_t mapstride=0;
  const uint8_t *map=fmn_map_get_view(&mapstride);
  if (map) {
    int16_t dsty=0;
    int16_t yi=FMN_SCREENH_TILES;
    const uint8_t *row=map;
    for (;yi-->0;dsty+=FMN_TILESIZE,row+=mapstride) {
      int16_t dstx=0;
      int16_t xi=FMN_SCREENW_TILES;
      const uint8_t *srcp=row;
      for (;xi-->0;dstx+=FMN_TILESIZE,srcp++) {
        int16_t srcx=((*srcp)&0x0f)*FMN_TILESIZE;
        int16_t srcy=((*srcp)>>4)*FMN_TILESIZE;
        fmn_blit(&bgbits,dstx,dsty,&bgtiles,srcx,srcy,FMN_TILESIZE,FMN_TILESIZE);
      }
    }
  } else {
    memset(bgbits.v,0,sizeof(bgbits_storage));
  }
}

/* Render.
 */
 
void fmn_play_render(struct fmn_image *fb) {

  // Rendering bgbits gets deferred to here, in case it gets multiple changes during an update.
  if (bgbitsdirty) {
    fmn_render_bgbits();
    bgbitsdirty=0;
  }
  
  // bgbits and fb must be exactly the same size and format.
  memcpy(fb->v,bgbits.v,sizeof(bgbits_storage));
}

/* Reset game.
 */
 
void fmn_game_reset() {
  fmn_map_reset();
  fmn_hero_reset();
}

void fmn_game_reset_with_password(uint32_t pw) {
  fmn_map_reset();
  fmn_hero_reset();
}

/* Generate password.
 */

uint32_t fmn_game_generate_password() {
  return 0;//TODO
}

/* Navigate.
 */

uint8_t fmn_game_navigate(int8_t dx,int8_t dy) {
  if (!fmn_map_navigate(dx,dy)) return 0;
  //TODO change sprites
  bgbitsdirty=1;
  return 1;
}
