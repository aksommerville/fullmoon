#include "game/fullmoon.h"
#include "game/fmn_title.h"
#include "game/fmn_data.h"

#define FMN_TITLE_SELECTION_NEW 0
#define FMN_TITLE_SELECTION_PASSWORD 1
#define FMN_TITLE_SELECTION_COUNT 2

/* Globals.
 */
 
static uint8_t fbdirty=0;
static uint8_t selection=FMN_TITLE_SELECTION_NEW;
static uint8_t arrowframe=0;
static uint8_t arrowanimtime=0;

/* Begin.
 */
 
void fmn_title_begin() {
  fbdirty=1;
}

/* End.
 */
 
void fmn_title_end() {
}

/* Input.
 */
 
void fmn_title_input(uint16_t input,uint16_t prev) {
  fbdirty=1;
  
  if ((input&FMN_BUTTON_UP)&&!(prev&FMN_BUTTON_UP)) {
    if (selection>0) selection--;
    else selection=FMN_TITLE_SELECTION_COUNT-1;
  }
  
  if ((input&FMN_BUTTON_DOWN)&&!(prev&FMN_BUTTON_DOWN)) {
    selection++;
    if (selection>=FMN_TITLE_SELECTION_COUNT) selection=0;
  }
  
  if (
    ((input&FMN_BUTTON_A)&&!(prev&FMN_BUTTON_A))||
    ((input&FMN_BUTTON_B)&&!(prev&FMN_BUTTON_B))
  ) switch (selection) {
    case FMN_TITLE_SELECTION_NEW: {
        fmn_game_reset();
        fmn_set_uimode(FMN_UIMODE_PLAY); 
      } break;
    case FMN_TITLE_SELECTION_PASSWORD: {
        fmn_set_uimode(FMN_UIMODE_PASSWORD);
      } break;
  }
}

/* Update.
 */
 
void fmn_title_update() {
  if (arrowanimtime) {
    arrowanimtime--;
  } else {
    arrowanimtime=7;
    arrowframe++;
    if (arrowframe>=6) arrowframe=0;
    fbdirty=1;
  }
}

/* Render.
 */
 
void fmn_title_render(struct fmn_image *fb) {
  if (!fbdirty) return;
  fmn_blit(fb,FMN_NSCOORD( 0, 0),&titlesplash,FMN_NSCOORD(0,0),fb->w,fb->h,0);
  fmn_blit(fb,FMN_NSCOORD(23,18),&uibits,FMN_NSCOORD(0,selection*12),FMN_NSCOORD(48,12),0);
  fmn_blit(fb,FMN_NSCOORD(44,13),&uibits,FMN_NSCOORD(48+arrowframe*7,0),FMN_NSCOORD(7,5),0);
  fmn_blit(fb,FMN_NSCOORD(44,30),&uibits,FMN_NSCOORD(48+arrowframe*7,5),FMN_NSCOORD(7,5),0);
  fbdirty=0;
}
