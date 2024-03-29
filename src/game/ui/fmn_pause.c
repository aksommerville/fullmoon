#include "game/fullmoon.h"
#include "game/ui/fmn_pause.h"
#include "game/fmn_data.h"
#include "game/sprite/hero/fmn_hero.h"
#include <string.h>

/* Globals.
 */
 
static uint8_t fbdirty=0;
static uint8_t cursorp=0;
static uint8_t cursorframe=0;
static uint8_t cursortime=0;
static char password[FMN_PASSWORD_LENGTH];
static int8_t password_error;
static uint16_t itemflags=0;

/* Begin.
 */
 
void fmn_pause_begin() {
  fbdirty=1;
  uint16_t state=fmn_game_get_state();
  password_error=fmn_password_repr(password,state);
  itemflags=state&(FMN_STATE_BROOM|FMN_STATE_FEATHER|FMN_STATE_WAND|FMN_STATE_UMBRELLA);
  
  if (password_error) {
    fprintf(stderr,"!!! %s: password_error=%d state=0x%04x\n",__func__,password_error,state);
  }
  
  // If at least one item is available, ensure something valid is selected.
  if (itemflags) {
    uint8_t action=fmn_hero_get_action();
    uint8_t showaction=fmn_pause_get_action();
    if (action==showaction) {
      // ok hero agrees with us. But if it is NONE, find a default.
      if (action==FMN_ACTION_NONE) {
        uint8_t i=0;
        for (;i<4;i++) {
          cursorp=i;
          if (action=fmn_pause_get_action()) {
            fmn_hero_set_action(action);
            return;
          }
        }
      }
    } else switch (action) {
      // hero disagrees with us. change our selection to match her.
      case FMN_ACTION_BROOM: cursorp=0; break;
      case FMN_ACTION_FEATHER: cursorp=1; break;
      case FMN_ACTION_WAND: cursorp=2; break;
      case FMN_ACTION_UMBRELLA: cursorp=3; break;
      // or if the hero has nothing selected, pick something for her.
      case FMN_ACTION_NONE: {
          uint8_t i=0;
          for (;i<4;i++) {
            cursorp=i;
            if (action=fmn_pause_get_action()) {
              fmn_hero_set_action(action);
              return;
            }
          }
        } break;
    }
  }
}

/* End.
 */
 
void fmn_pause_end() {
}

/* Get selection as action.
 */
 
uint8_t fmn_pause_get_action() {
  switch (cursorp) {
    case 0: return (itemflags&FMN_STATE_BROOM)?FMN_ACTION_BROOM:FMN_ACTION_NONE;
    case 1: return (itemflags&FMN_STATE_FEATHER)?FMN_ACTION_FEATHER:FMN_ACTION_NONE;
    case 2: return (itemflags&FMN_STATE_WAND)?FMN_ACTION_WAND:FMN_ACTION_NONE;
    case 3: return (itemflags&FMN_STATE_UMBRELLA)?FMN_ACTION_UMBRELLA:FMN_ACTION_NONE;
  }
  return FMN_ACTION_NONE;
}

uint8_t fmn_pause_get_verified_action() {
  itemflags=fmn_game_get_state()&(FMN_STATE_BROOM|FMN_STATE_FEATHER|FMN_STATE_WAND|FMN_STATE_UMBRELLA);
  return fmn_pause_get_action();
}

/* Move selection.
 */
 
static void fmn_pause_move(int8_t dx,int8_t dy) {
  if (dx<0) {
    if (cursorp) cursorp--;
    else cursorp=3;
  } else if (dx>0) {
    if (cursorp<3) cursorp++;
    else cursorp=0;
  } else return;
  fmn_hero_set_action(fmn_pause_get_action());
}

/* Input.
 */
 
void fmn_pause_input(uint16_t input,uint16_t prev) {
  if (!input) return; // we only do stateless inputs
  fbdirty=1;
  
  if (
    ((input&FMN_BUTTON_A)&&!(prev&FMN_BUTTON_A))||
    ((input&FMN_BUTTON_B)&&!(prev&FMN_BUTTON_B))
  ) {
    // Maybe other things depending on what's selected?
    fmn_set_uimode(FMN_UIMODE_PLAY);
  }
  
  if ((input&FMN_BUTTON_UP)&&!(prev&FMN_BUTTON_UP)) fmn_pause_move(0,-1);
  if ((input&FMN_BUTTON_DOWN)&&!(prev&FMN_BUTTON_DOWN)) fmn_pause_move(0,1);
  if ((input&FMN_BUTTON_LEFT)&&!(prev&FMN_BUTTON_LEFT)) fmn_pause_move(-1,0);
  if ((input&FMN_BUTTON_RIGHT)&&!(prev&FMN_BUTTON_RIGHT)) fmn_pause_move(1,0);
}

/* Update.
 */
 
void fmn_pause_update() {
  if (cursortime) cursortime--;
  else {
    cursortime=7;
    cursorframe++;
    if (cursorframe>=4) cursorframe=0;
    fbdirty=1;
  }
}

/* Render.
 */
 
void fmn_pause_render(struct fmn_image *fb) {
  if (!fbdirty) return;
  fbdirty=0;
  fmn_image_clear(fb);
  
  fmn_blit(fb,FMN_NSCOORD(cursorp*18,5),&uibits,FMN_NSCOORD(cursorframe*18,84),FMN_NSCOORD(18,18),0);
  
  uint8_t i=0;
  for (i=0;i<4;i++) {
    switch (i) {
      case 0: if (!(itemflags&FMN_STATE_BROOM)) continue; break;
      case 1: if (!(itemflags&FMN_STATE_FEATHER)) continue; break;
      case 2: if (!(itemflags&FMN_STATE_WAND)) continue; break;
      case 3: if (!(itemflags&FMN_STATE_UMBRELLA)) continue; break;
    }
    fmn_blit(fb,FMN_NSCOORD(1+i*18,6),&mainsprites,FMN_NSCOORD(i*16,32),FMN_NSCOORD(16,16),0);
  }
  
  if (!password_error) {
    fmn_blit(fb,FMN_NSCOORD(1,30),&uibits,FMN_NSCOORD(0,102),FMN_NSCOORD(34,7),0);
    for (i=0;i<FMN_PASSWORD_LENGTH;i++) {
      uint8_t chp;
      if ((password[i]>='A')&&(password[i]<='Z')) chp=password[i]-'A';
      else switch (password[i]) {
        case '1': chp=26; break;
        case '3': chp=27; break;
        case '4': chp=28; break;
        case '6': chp=29; break;
        case '7': chp=30; break;
        case '9': chp=31; break;
      }
      if (chp>=16) fmn_blit(fb,FMN_NSCOORD(36+i*6,30),&uibits,FMN_NSCOORD((chp-16)*5,116),FMN_NSCOORD(5,7),0);
      else fmn_blit(fb,FMN_NSCOORD(36+i*6,30),&uibits,FMN_NSCOORD(chp*5,109),FMN_NSCOORD(5,7),0);
    }
  }
}
