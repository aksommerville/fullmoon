#include "fullmoon.h"
#include "fmn_pause.h"
#include "fmn_data.h"
#include <string.h>

/* Globals.
 */
 
static uint8_t fbdirty=0;
static uint8_t cursorp=0;
static uint8_t cursorframe=0;
static uint8_t cursortime=0;
static uint8_t password[5];

/* Begin.
 */
 
void fmn_pause_begin() {
  fbdirty=1;
  uint32_t display=fmn_password_encode(fmn_game_generate_password());
  uint8_t i=0; for (;i<5;i++) password[i]=(display>>(i*5))&0x1f;
}

/* End.
 */
 
void fmn_pause_end() {
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
  }
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
  memset(fb->v,0,(fb->w*fb->h)>>3);
  
  fmn_blit(fb,cursorp*18,5,&uibits,cursorframe*18,94,18,18);
  
  uint8_t i=0;
  for (i=0;i<4;i++) fmn_blit(fb,1+i*18,6,&mainsprites,i*16,32,16,16);
  
  fmn_blit(fb,6,30,&uibits,0,112,34,7);
  for (i=0;i<5;i++) fmn_blit(fb,44+i*4,30,&uibits,password[i]*3,119,3,7);
}
