#include "game/fullmoon.h"
#include "game/ui/fmn_gameover.h"
#include "game/fmn_data.h"

#define FMN_GAMEOVER_BLACKOUT_TIME 60

/* Globals.
 */
 
static uint16_t clock=0;
static uint8_t cursorp=0; // 0,1,2 = none,continue,quit
static uint32_t pw_plain,pw_display;
static uint8_t password[5];
 
/* Begin.
 */
 
void fmn_gameover_begin() {
  clock=0;
  cursorp=0;
  pw_plain=fmn_game_generate_password();
  pw_display=fmn_password_encode(pw_plain);
  uint8_t i=0; for (;i<5;i++) password[i]=(pw_display>>(i*5))&0x1f;
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
        case 1: fmn_game_reset_with_password(pw_plain); fmn_set_uimode(FMN_UIMODE_PLAY); break;
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
  
  // Skull & crossbones.
  fmn_blit(fb,FMN_NSCOORD(36-13,0),&uibits,FMN_NSCOORD(72,24),FMN_NSCOORD(26,22),0);
  
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
  fmn_blit(fb,FMN_NSCOORD(6,23),&uibits,FMN_NSCOORD(0,112),FMN_NSCOORD(34,7),0);
  uint8_t i;
  for (i=0;i<5;i++) fmn_blit(fb,FMN_NSCOORD(46+i*4,23),&uibits,FMN_NSCOORD(password[i]*3,119),FMN_NSCOORD(3,7),0);
}
