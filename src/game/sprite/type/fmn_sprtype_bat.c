/* fmn_sprtype_bat.c
 */

#include "game/fullmoon.h"
#include "game/model/fmn_map.h"
#include "game/sprite/hero/fmn_hero.h"
#include "game/sprite/fmn_sprite.h"
#include <stdio.h>

#define frame sprite->bv[0]
#define frametime sprite->bv[1]
#define phase sprite->bv[2]
#define homex sprite->sv[0]
#define homey sprite->sv[1]
#define radius sprite->sv[2]
#define radiusd sprite->sv[3]

#define FMN_BAT_RADIUS_SPEED 1
#define FMN_BAT_RADIUS_MIN 0
#define FMN_BAT_RADIUS_MAX (FMN_MM_PER_TILE*3)
#define FMN_BAT_PHASE_SPEED 4
#define FMN_BAT_MOVE_SPEED 3

static void _bat_del(struct fmn_sprite *sprite) {
}

static int8_t _bat_init(struct fmn_sprite *sprite,const struct fmn_sprdef *sprdef) {
  homex=sprite->x;
  homey=sprite->y;
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  sprite->layer=50;
  radius=0;
  radiusd=1;
  return 0;
}

static void fmn_bat_move(struct fmn_sprite *sprite) {
  /* Bat makes a dizzy little spiral by tracking a point that move along a diamond centered on his home point.
   * The radius of that diamond is constantly oscillating.
   */
  radius+=radiusd*FMN_BAT_RADIUS_SPEED;
  if ((radius>FMN_BAT_RADIUS_MAX)&&(radiusd>0)) radiusd=-radiusd;
  else if ((radius<FMN_BAT_RADIUS_MIN)&&(radiusd<0)) radiusd=-radiusd;
  phase+=FMN_BAT_PHASE_SPEED;
  int16_t targetx,targety;
  if (phase<0x40) { // NE
    targetx=homex+(radius*(phase&0x3f))/0x40;
    targety=homey-radius+(radius*(phase&0x3f))/0x40;
  } else if (phase<0x80) { // SE
    targetx=homex+radius-(radius*(phase&0x3f))/0x40;
    targety=homey+(radius*(phase&0x3f))/0x40;
  } else if (phase<0xc0) { // SW
    targetx=homex-(radius*(phase&0x3f))/0x40;
    targety=homey+radius-(radius*(phase&0x3f))/0x40;
  } else { // NW
    targetx=homex-radius+(radius*(phase&0x3f))/0x40;
    targety=homey-(radius*(phase&0x3f))/0x40;
  }
  targetx-=sprite->w>>1;
  targety-=sprite->h>>1;
  if (sprite->x>targetx+FMN_BAT_MOVE_SPEED) sprite->x-=FMN_BAT_MOVE_SPEED;
  else if (sprite->x<targetx-FMN_BAT_MOVE_SPEED) sprite->x+=FMN_BAT_MOVE_SPEED;
  else sprite->x=targetx;
  if (sprite->y>targety+FMN_BAT_MOVE_SPEED) sprite->y-=FMN_BAT_MOVE_SPEED;
  else if (sprite->y<targety-FMN_BAT_MOVE_SPEED) sprite->y+=FMN_BAT_MOVE_SPEED;
  else sprite->y=targety;
}

static void fmn_bat_animate(struct fmn_sprite *sprite) {
  if (frametime>0) {
    frametime--;
  } else {
    frametime=5;
    frame++;
    if (frame>=6) frame=0;
    switch (frame) {
      case 0: sprite->tileid=0x81; break;
      case 1: sprite->tileid=0x82; break;
      case 2: sprite->tileid=0x83; break;
      case 3: sprite->tileid=0x84; break;
      case 4: sprite->tileid=0x83; break;
      case 5: sprite->tileid=0x82; break;
    }
  }
}

static void fmn_bat_check_hero(struct fmn_sprite *sprite) {
  int16_t midx=sprite->x+(sprite->w>>1);
  int16_t midy=sprite->y+(sprite->h>>1);
  int16_t herox,heroy,herow,heroh;
  fmn_hero_get_world_bounds(&herox,&heroy,&herow,&heroh);
  if (midx<herox) return;
  if (midy<heroy) return;
  if (midx>=herox+herow) return;
  if (midy>=heroy+heroh) return;
  fmn_hero_injure(sprite);
}

static void _bat_update(struct fmn_sprite *sprite) {
  fmn_bat_move(sprite);
  fmn_bat_animate(sprite);
  fmn_bat_check_hero(sprite);
}

const struct fmn_sprtype fmn_sprtype_bat={
  .name="bat",
  .del=_bat_del,
  .init=_bat_init,
  .update=_bat_update,
  .render=fmn_sprite_render_default,
};
