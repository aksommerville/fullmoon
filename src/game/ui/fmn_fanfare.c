#include "game/fullmoon.h"
#include "game/fmn_data.h"
#include "game/ui/fmn_fanfare.h"

/* Globals
 */
 
static uint8_t fanfare_action=0;
static uint16_t fanfare_framec=0;

// We do a keydown then wait for keyup, in order to ensure we return to play with a zero input state.
static uint8_t fanfare_input_delay=0; // nonzero initially, until input goes zero
static uint8_t fanfare_input_hold=0; // nonzero when the dismiss key is held -- wait for it to go zero

#define FANFARE_TIME_INPUT_ENABLE 60
#define FANFARE_TIME_CURTAINS_START 30
#define FANFARE_TIME_CURTAINS_END 90

#define FANFARE_CURTAINS_DISPLACEMENT ((FMN_FBW>>1)-(5*FMN_GFXSCALE))
 
/* Begin
 */
 
void fmn_fanfare_begin() {
  fanfare_framec=0;
  fanfare_input_delay=1;
  fanfare_input_hold=0;
}

void fmn_fanfare_set_action(uint8_t action) {
  fanfare_action=action;
}

/* End
 */
 
void fmn_fanfare_end() {
}

/* Input
 */
 
void fmn_fanfare_input(uint16_t input,uint16_t pvinput) {

  // It's possible that all input bits were off when we launched -- to detect that, drop the hold when pvinput==0.
  if (!input||!pvinput) fanfare_input_delay=0;
  
  if (fanfare_framec>=FANFARE_TIME_INPUT_ENABLE) {
    if (!input&&fanfare_input_hold) {
      fmn_set_uimode(FMN_UIMODE_PLAY);
    } else if (
      ((input&FMN_BUTTON_A)&&!(pvinput&FMN_BUTTON_A))||
      ((input&FMN_BUTTON_B)&&!(pvinput&FMN_BUTTON_B))
    ) {
      fanfare_input_hold=1;
    }
  }
}

/* Update
 */
 
void fmn_fanfare_update() {
  fanfare_framec++;
}

/* Render.
 */
 
void fmn_fanfare_render(struct fmn_image *fb) {
  fmn_image_clear(fb);
  
  // treasure and sparkles
  if (fanfare_framec>=FANFARE_TIME_CURTAINS_START) {
    int16_t srcw=FMN_TILESIZE*2,srch=FMN_TILESIZE*2;
    int16_t srcx=(fanfare_action-1)*srcw;
    int16_t srcy=FMN_TILESIZE*4;
    fmn_blit(fb,(fb->w>>1)-(srcw>>1),(fb->h>>1)-(srch>>1)+5*FMN_GFXSCALE,&mainsprites,srcx,srcy,srcw,srch,0);
    
    uint8_t sparkleframe=(fanfare_framec/4)%7;
    srcw=7*FMN_GFXSCALE;
    srch=13*FMN_GFXSCALE;
    if (sparkleframe>=4) {
      srcx=(36+7*(sparkleframe-4))*FMN_GFXSCALE;
      srcy=53*FMN_GFXSCALE;
    } else {
      srcx=(36+7*sparkleframe)*FMN_GFXSCALE;
      srcy=40*FMN_GFXSCALE;
    }
    fmn_blit(fb,FMN_NSCOORD(14,18),&fanfare,srcx,srcy,srcw,srch,0);
    fmn_blit(fb,FMN_NSCOORD(50,18),&fanfare,srcx,srcy,srcw,srch,FMN_XFORM_XREV);
  }
  
  // curtains
  int16_t curtaind;
  if (fanfare_framec<=FANFARE_TIME_CURTAINS_START) curtaind=0;
  else if (fanfare_framec>=FANFARE_TIME_CURTAINS_END) curtaind=FANFARE_CURTAINS_DISPLACEMENT;
  else curtaind=((fanfare_framec-FANFARE_TIME_CURTAINS_START)*FANFARE_CURTAINS_DISPLACEMENT)/(FANFARE_TIME_CURTAINS_END-FANFARE_TIME_CURTAINS_START);
  fmn_blit(fb,-curtaind,0,&fanfare,FMN_NSCOORD(0,40),FMN_NSCOORD(36,40),0);
  fmn_blit(fb,(FMN_FBW>>1)+curtaind,0,&fanfare,FMN_NSCOORD(0,40),FMN_NSCOORD(36,40),FMN_XFORM_XREV);
  
  // bunting
  fmn_blit(fb,0,0,&fanfare,0,0,FMN_FBW,10*FMN_GFXSCALE,0);
}
