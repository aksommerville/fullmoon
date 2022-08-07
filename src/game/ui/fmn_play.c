#include "game/fullmoon.h"
#include "game/ui/fmn_play.h"
#include "game/fmn_data.h"
#include "game/model/fmn_map.h"
#include "game/model/fmn_proximity.h"
#include "game/sprite/fmn_sprite.h"
#include "game/sprite/hero/fmn_hero.h"
#include <string.h>

/* Globals.
 */
 
static uint8_t bgbits_storage[FMN_FB_SIZE_BYTES];
static struct fmn_image bgbits={
  .v=bgbits_storage,
  .w=FMN_FBW,
  .h=FMN_FBH,
  .stride=FMN_FB_STRIDE,
  .fmt=FMN_FBFMT,
  .writeable=1,
};
static uint8_t bgbitsdirty=1;

static uint16_t raintime=0;
static uint16_t statebits=0;
static uint16_t gameovertime=0; // if nonzero counts down to gameover

uint32_t fmn_play_frame_count=0;
 
/* Begin.
 * Mind that this only means "ui mode changed"; not necessarily "start a fresh game".
 * There will always be fmn_game_reset() or fmn_game_reset_with_state() before the first fmn_play_begin().
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
  int16_t scrollx,scrolly;
  fmn_map_get_scroll_mm(&scrollx,&scrolly);
  struct fmn_sprite **p=fmn_spritev;
  uint16_t i=fmn_spritec;
  for (;i-->0;p++) {
    struct fmn_sprite *sprite=*p;
    if (!(sprite->flags&FMN_SPRITE_FLAG_RAINABLE)) continue;
    if (sprite->x>=scrollx+FMN_SCREENW_MM) continue;
    if (sprite->y>=scrolly+FMN_SCREENH_MM) continue;
    if (sprite->x+sprite->w<=scrollx) continue;
    if (sprite->y+sprite->h<=scrolly) continue;
    fmn_sprite_del_later(sprite);
  }
}

/* Update.
 */
 
void fmn_play_update() {
  fmn_play_frame_count++;

  fmn_sprites_update();

  if (fmn_hero_get_sprite()) {
    int16_t x,y;
    fmn_hero_get_world_position_center(&x,&y);
    fmn_map_update(x,y);
    fmn_proximity_update(x,y);
  }
  
  fmn_sprites_execute_deathrow();
  
  if (raintime>1) raintime--;
  else if (raintime==1) {
    raintime=0;
    fmn_finish_rain();
  }
  
  if (gameovertime) {
    if (!--gameovertime) {
      fmn_set_uimode(FMN_UIMODE_GAMEOVER);
    }
  }
}

/* Render bgbits.
 */
 
void fmn_bgbits_dirty() {
  bgbitsdirty=1;
}
 
static void fmn_render_bgbits() {
  const struct fmn_map *mapobj=fmn_map_get();
  const struct fmn_image *srcimg=(mapobj?mapobj->tilesheet:0);
  uint8_t mapstride=0;
  const uint8_t *map=fmn_map_get_view(&mapstride);
  if (map&&srcimg) {
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
        fmn_blit(&bgbits,dstx,dsty,srcimg,srcx,srcy,FMN_TILESIZE,FMN_TILESIZE,0);
      }
    }
  } else {
    memset(bgbits.v,0,sizeof(bgbits_storage));
  }
}

/* Render rain.
 */
 
static const int16_t rainseed[9]={
  -3*FMN_GFXSCALE,
  -7*FMN_GFXSCALE,
  -1*FMN_GFXSCALE,
  -5*FMN_GFXSCALE,
  -7*FMN_GFXSCALE,
  -4*FMN_GFXSCALE,
  -6*FMN_GFXSCALE,
  -2*FMN_GFXSCALE,
  -0*FMN_GFXSCALE,
};

static void render_rain(struct fmn_image *fb) {
  int16_t dstx=0;
  uint8_t xi=FMN_SCREENW_TILES;
  for (;xi-->0;dstx+=FMN_TILESIZE) {
    int16_t dsty=rainseed[xi]-(raintime%FMN_TILESIZE);
    for (;dsty<FMN_FBH;dsty+=FMN_TILESIZE) {
      fmn_blit_tile(fb,dstx,dsty,&mainsprites,0x27,0);
    }
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
  
  fmn_blit(fb,0,0,&bgbits,0,0,FMN_FBW,FMN_FBH,0);
  
  // Sprites.
  fmn_sprites_render(fb);
  
  // Rain.
  if (raintime) render_rain(fb);
  
  //TODO additional overlay?
}

/* Reset game.
 */
 
void fmn_game_reset() {
  fmn_play_frame_count=0;
  raintime=0;
  statebits=0;
  memset((void*)fmn_gstate,0,sizeof(fmn_gstate));
  fmn_map_reset();
  fmn_hero_reset();
}

void fmn_game_reset_with_state(uint16_t state) {
  fmn_play_frame_count=0;

  statebits=state;
  
  memset((void*)fmn_gstate,0,sizeof(fmn_gstate));
  if (state&FMN_STATE_TUNNEL_SWITCH) fmn_gstate[FMN_GSTATE_tunnel_switch]=1;

  raintime=0;
  fmn_map_reset_region((state&FMN_STATE_LOCATION_MASK)>>FMN_STATE_LOCATION_SHIFT);
  fmn_hero_reset();
}

/* Generate password.
 */

uint16_t fmn_game_get_state() {
  uint16_t state=(statebits&~FMN_STATE_LOCATION_MASK)|(fmn_map_get_region()<<FMN_STATE_LOCATION_SHIFT);
  
  #define GSTATE(p,mask) if (fmn_gstate[p]) state|=mask; else state&=~mask;
  GSTATE(FMN_GSTATE_tunnel_switch,FMN_STATE_TUNNEL_SWITCH)
  #undef GSTATE
  
  return state;
}

void fmn_game_set_state(uint16_t mask,uint16_t value) {
  if (value&~mask) return; // invalid
  
  // The fmn_game_get_state() here ensures that we pick up changes to fmn_gstate made behind our back.
  statebits=(fmn_game_get_state()&~mask)|value;
  
  #define GSTATE(p,m) fmn_gstate[p]=(statebits&m)?1:0;
  GSTATE(FMN_GSTATE_tunnel_switch,FMN_STATE_TUNNEL_SWITCH)
  #undef GSTATE
}

/* Spells.
 */
 
static void fmn_spell_open() {
  int16_t scrollx,scrolly;
  fmn_map_get_scroll_mm(&scrollx,&scrolly);
  struct fmn_sprite **p=fmn_spritev;
  uint16_t i=fmn_spritec;
  for (;i-->0;p++) {
    struct fmn_sprite *sprite=*p;
    if (!(sprite->flags&FMN_SPRITE_FLAG_OPENABLE)) continue;
    if (sprite->x>=scrollx+FMN_SCREENW_MM) continue;
    if (sprite->y>=scrolly+FMN_SCREENH_MM) continue;
    if (sprite->x+sprite->w<=scrollx) continue;
    if (sprite->y+sprite->h<=scrolly) continue;
    fmn_sprite_del_later(sprite);
  }
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

/* End game.
 */
 
void fmn_game_end(int16_t x,int16_t y) {
  gameovertime=180;
  if (x||y) {
    // Generate the seven circles of a witch's soul, and send them off to their reward.
    uint8_t i=7; while (i-->0) {
      struct fmn_sprite *sprite=fmn_sprite_new(&fmn_sprtype_soulball,0,x,y,i,7,0);
    }
  }
}
