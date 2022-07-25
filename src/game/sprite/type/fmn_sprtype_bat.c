/* fmn_sprtype_bat.c
 *
 * bv[0]: frame 0..6
 * bv[1]: frametime
 * sv[0]: dx
 * sv[1]: dy
 */

#include "game/fullmoon.h"
#include "game/model/fmn_map.h"
#include "game/sprite/fmn_sprite.h"
#include <stdlib.h>

static void _bat_del(struct fmn_sprite *sprite) {
}

static int8_t _bat_init(struct fmn_sprite *sprite,const struct fmn_sprdef *sprdef) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->bv[0]=rand()%6;
  sprite->bv[1]=rand()%10;
  sprite->layer=50;
//XXX highly temporary. Random position and velocity.
  int16_t worldw,worldh;
  fmn_map_get_size_mm(&worldw,&worldh);
  sprite->x=rand()%(worldw-FMN_MM_PER_TILE);
  sprite->y=rand()%(worldh-FMN_MM_PER_TILE);
  sprite->sv[0]=rand()%7-3;
  sprite->sv[1]=rand()%7-3;
  if (sprite->sv[0]<0) sprite->sv[0]-=1;
  else sprite->sv[0]+=1;
  if (sprite->sv[1]<0) sprite->sv[1]-=1;
  else sprite->sv[1]+=1;
  return 0;
}

static void _bat_update(struct fmn_sprite *sprite) {
  int16_t worldw,worldh;
  fmn_map_get_size_mm(&worldw,&worldh);

  // update position and velocity -- bounce off the world edges
  sprite->x+=sprite->sv[0];
  if (
    ((sprite->x<0)&&(sprite->sv[0]<0))||
    ((sprite->x+sprite->w>worldw)&&(sprite->sv[0]>0))
  ) sprite->sv[0]*=-1;
  sprite->y+=sprite->sv[1];
  if (
    ((sprite->y<0)&&(sprite->sv[1]<0))||
    ((sprite->y+sprite->h>worldh)&&(sprite->sv[1]>0))
  ) sprite->sv[1]*=-1;
  
  // animate
  if (sprite->bv[1]>0) sprite->bv[1]--;
  else {
    sprite->bv[1]=5;
    sprite->bv[0]++;
    if (sprite->bv[0]>=6) sprite->bv[0]=0;
    switch (sprite->bv[0]) {
      case 0: sprite->tileid=0x81; break;
      case 1: sprite->tileid=0x82; break;
      case 2: sprite->tileid=0x83; break;
      case 3: sprite->tileid=0x84; break;
      case 4: sprite->tileid=0x83; break;
      case 5: sprite->tileid=0x82; break;
    }
  }
}

const struct fmn_sprtype fmn_sprtype_bat={
  .name="bat",
  .del=_bat_del,
  .init=_bat_init,
  .update=_bat_update,
  .render=fmn_sprite_render_default,
  .hitbox=fmn_sprite_hitbox_all,
};
