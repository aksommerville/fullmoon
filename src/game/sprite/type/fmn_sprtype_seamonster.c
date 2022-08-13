#include "game/fullmoon.h"
#include "game/ui/fmn_play.h"
#include "game/model/fmn_map.h"
#include "game/sprite/hero/fmn_hero.h"
#include "game/sprite/fmn_sprite.h"
#include <math.h>

#define animclock sprite->bv[3]
#define animframe sprite->bv[4]
#define stage sprite->bv[5]
#define basetileid sprite->bv[6]
#define stagetime sprite->sv[0]
#define dx sprite->sv[1]
#define dy sprite->sv[2]

#define FMN_SEAMONSTER_STAGE_SWIM 0
#define FMN_SEAMONSTER_STAGE_MENACE 1
#define FMN_SEAMONSTER_STAGE_BREATHE 2
#define FMN_SEAMONSTER_STAGE_WRAPUP 3

#define FMN_SEAMONSTER_SWIM_TIME 200
#define FMN_SEAMONSTER_MENACE_TIME 30
#define FMN_SEAMONSTER_BREATHE_TIME 30
#define FMN_SEAMONSTER_WRAPUP_TIME 60
#define FMN_SEAMONSTER_FIREBALL_SPEED 6
#define FMN_SEAMONSTER_SWIM_SPEED 2

/* Set swim direction based on low bits of the global clock.
 */
 
static void fmn_seamonster_randomish_direction(struct fmn_sprite *sprite) {
  switch (fmn_play_frame_count&3) {
    case 0: dx=0; dy=-1; break;
    case 1: dx=0; dy=1; break;
    case 2: dx=-1; dy=0; break;
    case 3: dx=1; dy=0; break;
  }
}

/* Setup.
 */
 
static int8_t _seamonster_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  stage=FMN_SEAMONSTER_STAGE_SWIM;
  stagetime=FMN_SEAMONSTER_SWIM_TIME;
  basetileid=sprite->tileid;
  fmn_seamonster_randomish_direction(sprite);
  return 0;
}

/* Enter BREATHE stage; create a fireball if onscreen.
 */
 
static void fmn_seamonster_begin_BREATHE(struct fmn_sprite *sprite) {
  stage=FMN_SEAMONSTER_STAGE_BREATHE;
  stagetime=FMN_SEAMONSTER_BREATHE_TIME;
  
  int16_t scrollx,scrolly;
  fmn_map_get_scroll_mm(&scrollx,&scrolly);
  if (sprite->x+sprite->w<=scrollx) return;
  if (sprite->y+sprite->h<=scrolly) return;
  if (sprite->x>=scrollx+FMN_SCREENW_MM) return;
  if (sprite->y>=scrolly+FMN_SCREENH_MM) return;
  
  int16_t x=sprite->x;
  if (!(sprite->xform&FMN_XFORM_XREV)) x+=sprite->w;
  int16_t y=sprite->y+(sprite->h>>1);
  struct fmn_sprite *fireball=fmn_sprite_new(&fmn_sprtype_missile,0,x,y,0,0,0);
  if (fireball) {
    // (sv[1],sv[2]) are (dx,dy); bv[0] is animclock, which we must set to enable
    fireball->image=sprite->image;
    fireball->tileid=0x38;
    fireball->bv[0]=1;
    int16_t herox,heroy;
    fmn_hero_get_world_position_center(&herox,&heroy);
    float distance=sqrtf((herox-x)*(herox-x)+(heroy-y)*(heroy-y));
    fireball->sv[1]=((herox-x)*FMN_SEAMONSTER_FIREBALL_SPEED)/distance;
    fireball->sv[2]=((heroy-y)*FMN_SEAMONSTER_FIREBALL_SPEED)/distance;
  }
}

/* Turn to face the hero.
 */
 
static void fmn_seamonster_face_hero(struct fmn_sprite *sprite) {
  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  if (herox<sprite->x) sprite->xform=FMN_XFORM_XREV;
  else sprite->xform=0;
}

/* A seamonster must rest entirely on HOLE tiles.
 * We can't use fmn_sprite_collide() straight for this as there isn't a "not hole" flag.
 */
 
static int8_t fmn_seamonster_out_of_water_cb(uint8_t props,void *userdata) {
  if (!(props&FMN_TILE_HOLE)) return 1;
  return 0;
}
 
static uint8_t fmn_seamonster_out_of_water(struct fmn_sprite *sprite) {
  return fmn_map_for_cell_props_in_rect(sprite->x,sprite->y,sprite->w,sprite->h,fmn_seamonster_out_of_water_cb,sprite);
}

/* Update in SWIM stage.
 */
 
static void fmn_seamonster_update_SWIM(struct fmn_sprite *sprite) {
  if (!stagetime--) {
    stage=FMN_SEAMONSTER_STAGE_MENACE;
    stagetime=FMN_SEAMONSTER_MENACE_TIME;
    return;
  }
  if (animclock) animclock--;
  else {
    animclock=14;
    animframe++;
    if (animframe>=2) animframe=0;
  }
  sprite->tileid=basetileid+animframe*0x10;
  
  sprite->x+=dx*FMN_SEAMONSTER_SWIM_SPEED;
  sprite->y+=dy*FMN_SEAMONSTER_SWIM_SPEED;
  if (fmn_sprite_collide(0,0,sprite,FMN_TILE_SOLID,0,1)) {
    fmn_seamonster_randomish_direction(sprite);
  } else if (fmn_seamonster_out_of_water(sprite)) {
    sprite->x-=dx*FMN_SEAMONSTER_SWIM_SPEED;
    sprite->y-=dy*FMN_SEAMONSTER_SWIM_SPEED;
    fmn_seamonster_randomish_direction(sprite);
  }
}

/* Update.
 */
 
static void _seamonster_update(struct fmn_sprite *sprite) {
  switch (stage) {
    case FMN_SEAMONSTER_STAGE_SWIM: fmn_seamonster_update_SWIM(sprite); break;
    case FMN_SEAMONSTER_STAGE_MENACE: {
        fmn_seamonster_face_hero(sprite);
        sprite->tileid=basetileid+0x01;
        if (!stagetime--) fmn_seamonster_begin_BREATHE(sprite);
      } break;
    case FMN_SEAMONSTER_STAGE_BREATHE: {
        sprite->tileid=basetileid+0x11;
        if (!stagetime--) {
          stage=FMN_SEAMONSTER_STAGE_WRAPUP;
          stagetime=FMN_SEAMONSTER_WRAPUP_TIME;
        }
      } break;
    case FMN_SEAMONSTER_STAGE_WRAPUP: {
        sprite->tileid=basetileid+0x01;
        if (!stagetime--) {
          stage=FMN_SEAMONSTER_STAGE_SWIM;
          // fudge the swim time by an unpredictable small count, to help "randomize" direction choices.
          int16_t herox,heroy;
          fmn_hero_get_world_position_center(&herox,&heroy);
          stagetime=FMN_SEAMONSTER_SWIM_TIME+((herox^heroy)%7);
          fmn_seamonster_randomish_direction(sprite);
        }
      } break;
  }
}

/* Type definition.
 */
 
const struct fmn_sprtype fmn_sprtype_seamonster={
  .name="seamonster",
  .init=_seamonster_init,
  .update=_seamonster_update,
  .render=fmn_sprite_render_default,
};
