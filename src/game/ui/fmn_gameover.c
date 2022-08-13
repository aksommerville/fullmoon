#include "game/fullmoon.h"
#include "game/ui/fmn_gameover.h"
#include "game/fmn_data.h"

#define FMN_GAMEOVER_BLACKOUT_TIME 60

/* Globals.
 */
 
static uint16_t clock=0;
static uint8_t cursorp=0; // 0,1,2 = none,continue,quit
static char password[FMN_PASSWORD_LENGTH];
static int8_t password_error;
static uint8_t disposition;
 
/* Begin.
 */
 
void fmn_gameover_begin() {
  clock=0;
  cursorp=0;
  password_error=fmn_password_repr(password,fmn_game_get_state());
  // It would be a really big deal if password_error were ever nonzero.
  // We do what we can at least, if it's invalid just don't show a password.
  disposition=FMN_GAMEOVER_DISPOSITION_DEAD;
}

void fmn_gameover_set_disposition(uint8_t d) {
  disposition=d;
}

/* End.
 */
 
void fmn_gameover_end() {
}

/* Input.
 */
 
void fmn_gameover_input(uint16_t input,uint16_t pvinput) {
  if (clock>=FMN_GAMEOVER_BLACKOUT_TIME) {
    #define _(tag) ((input&FMN_BUTTON_##tag)&&!(pvinput&FMN_BUTTON_##tag))
    if (_(LEFT)) cursorp=1;
    else if (_(RIGHT)) cursorp=2;
    else if (_(A)||_(B)) {
      switch (cursorp) {
        case 1: {
            if (password_error) fmn_game_reset();
            else fmn_game_reset_with_state(fmn_game_get_state());
            fmn_set_uimode(FMN_UIMODE_PLAY);
          } break;
        case 2: fmn_set_uimode(FMN_UIMODE_TITLE); break;
      }
    }
    #undef _
  }
}

/* Update.
 */
 
void fmn_gameover_update() {
  clock++;
}

/* Render.
 */
 
void fmn_gameover_render(struct fmn_image *fb) {
  fmn_image_clear(fb);
  
  uint8_t cursorframe=(clock/5)&3;
  int16_t cursorsrcx=113;
  int16_t cursorsrcy=33;
  if (cursorframe==3) cursorsrcx+=5;
  else cursorsrcx+=cursorframe*5;
  
  // Skull & crossbones or "The End".
  if (disposition==FMN_GAMEOVER_DISPOSITION_VICTORY) {
    fmn_blit(fb,FMN_NSCOORD(36-15,0),&uibits,FMN_NSCOORD(98,42),FMN_NSCOORD(30,22),0);
  } else {
    fmn_blit(fb,FMN_NSCOORD(36-13,0),&uibits,FMN_NSCOORD(72,24),FMN_NSCOORD(26,22),0);
  }
  
  if (clock>=FMN_GAMEOVER_BLACKOUT_TIME) {
    // "Continue"
    fmn_blit(fb,FMN_NSCOORD(7,31),&uibits,FMN_NSCOORD(98,24),FMN_NSCOORD(29,9),0);
    if (cursorp==1) {
      fmn_blit(fb,FMN_NSCOORD(1,31),&uibits,FMN_NSCOORD(cursorsrcx,cursorsrcy),FMN_NSCOORD(5,9),0);
      fmn_blit(fb,FMN_NSCOORD(37,31),&uibits,FMN_NSCOORD(cursorsrcx,cursorsrcy),FMN_NSCOORD(5,9),FMN_XFORM_XREV);
    }
  
    // "Quit"
    fmn_blit(fb,FMN_NSCOORD(50,31),&uibits,FMN_NSCOORD(98,33),FMN_NSCOORD(15,9),0);
    if (cursorp==2) {
      fmn_blit(fb,FMN_NSCOORD(44,31),&uibits,FMN_NSCOORD(cursorsrcx,cursorsrcy),FMN_NSCOORD(5,9),0);
      fmn_blit(fb,FMN_NSCOORD(66,31),&uibits,FMN_NSCOORD(cursorsrcx,cursorsrcy),FMN_NSCOORD(5,9),FMN_XFORM_XREV);
    }
  }
  
  // Password
  if (!password_error) {
    fmn_blit(fb,FMN_NSCOORD(1,23),&uibits,FMN_NSCOORD(0,102),FMN_NSCOORD(34,7),0);
    uint8_t i;
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
      if (chp>=16) fmn_blit(fb,FMN_NSCOORD(36+i*6,23),&uibits,FMN_NSCOORD((chp-16)*5,116),FMN_NSCOORD(5,7),0);
      else fmn_blit(fb,FMN_NSCOORD(36+i*6,23),&uibits,FMN_NSCOORD(chp*5,109),FMN_NSCOORD(5,7),0);
    }
  }
}
