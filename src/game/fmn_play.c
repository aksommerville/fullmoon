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

static uint16_t raintime=0;
 
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
  fmn_hero_set_input(0,0,0);
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

/* Finish the rain spell.
 */
 
static void fmn_finish_rain() {
  //TODO ok now what? what does the rain spell do?
}

/* Update.
 */
 
void fmn_play_update() {
  fmn_hero_update();
  
  if (raintime>1) raintime--;
  else if (raintime==1) {
    raintime=0;
    fmn_finish_rain();
  }
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
 
static const uint8_t rainseed[]={ 0,5,2,1,3,7,4,6, 3,6,2,1,5,4,7,0, 5,2,4,3,0,6,7,1, };
 
void fmn_play_render(struct fmn_image *fb) {

  // Rendering bgbits gets deferred to here, in case it gets multiple changes during an update.
  if (bgbitsdirty) {
    fmn_render_bgbits();
    bgbitsdirty=0;
  }
  
  // bgbits and fb must be exactly the same size and format.
  memcpy(fb->v,bgbits.v,sizeof(bgbits_storage));
  
  //TODO render sprites below hero
  fmn_hero_render(fb);
  //TODO render sprites above hero
  
  // Rain.
  if (raintime) {
    uint8_t *row=fb->v;
    uint8_t yi=5; for (;yi-->0;row+=fb->stride) {
      uint8_t seedp=0;
      uint8_t *p=row;
      uint8_t xi=fb->w>>1;
      for (;xi-->0;p+=2) {
        uint8_t mask=0x80>>((rainseed[seedp]+(raintime/2))&7);
        if (mask==0x80) mask|=1; else mask|=mask<<1;
        if (xi&1) (*p)&=~mask;
        else (*p)|=mask;
        seedp++;
        if (seedp>=sizeof(rainseed)) seedp=0;
      }
    }
  }
  
  //TODO additional overlay?
}

/* Reset game.
 */
 
void fmn_game_reset() {
  raintime=0;
  fmn_map_reset();
  fmn_hero_reset();
}

void fmn_game_reset_with_password(uint32_t pw) {
  raintime=0;
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

uint8_t fmn_game_focus_mm(int16_t xmm,int16_t ymm) {
  int8_t x=xmm/FMN_MM_PER_TILE;
  int8_t y=ymm/FMN_MM_PER_TILE;
  uint8_t vx,vy;
  fmn_map_get_scroll(&vx,&vy);
  // We'll only change one thing at a time.
  if (x<vx) return fmn_game_navigate(-1,0);
  if (y<vy) return fmn_game_navigate(0,-1);
  if (x>=vx+FMN_SCREENW_TILES) return fmn_game_navigate(1,0);
  if (y>=vy+FMN_SCREENH_TILES) return fmn_game_navigate(0,1);
  return 0;
}

/* Spells.
 */
 
static void fmn_spell_open() {
  fprintf(stderr,"%s\n",__func__);
}
 
static void fmn_spell_rain() {
  // Maybe check if we're indoors? Like, certain maps should be "rainproof".
  raintime=180;
}

/* Cast a spell.
 */
 
uint8_t fmn_game_cast_spell(const uint8_t *src,uint8_t srcc) {
  #define CHECKSPELL(fn,...) { \
    const uint8_t expect[]={__VA_ARGS__}; \
    if ((srcc==sizeof(expect))&&!memcmp(src,expect,srcc)) { \
      fn(); \
      return 1; \
    } \
  }
  CHECKSPELL(fmn_spell_open,FMN_DIR_W,FMN_DIR_E,FMN_DIR_W,FMN_DIR_N,FMN_DIR_N)
  CHECKSPELL(fmn_spell_rain,FMN_DIR_N,FMN_DIR_S,FMN_DIR_S,FMN_DIR_S)
  //TODO spells. i want like a dozen
  #undef CHECKSPELL
  return 0;
}
