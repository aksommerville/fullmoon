#include "fullmoon.h"
#include "fmn_play.h"
#include "fmn_data.h"
#include "game/model/fmn_hero.h"
#include "game/model/fmn_map.h"
#include <string.h>

/* Globals.
 */
 
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

}

/* Update.
 */
 
void fmn_play_update() {
}

/* Render.
 */
 
void fmn_play_render(struct fmn_image *fb) {
  memset(fb->v,0xaa,(fb->w*fb->h)>>3);//TODO
}

/* Reset game.
 */
 
void fmn_game_reset() {
  fmn_hero_reset();
}

void fmn_game_reset_with_password(uint32_t pw) {
  fmn_hero_reset();
}

/* Generate password.
 */

uint32_t fmn_game_generate_password() {
  return 0;//TODO
}
