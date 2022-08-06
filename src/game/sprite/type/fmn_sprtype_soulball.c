#include "game/fullmoon.h"
#include "game/fmn_data.h"
#include "game/sprite/fmn_sprite.h"
#include <stdio.h>
#include <math.h>

// Soulballs always travel in packs. We start up with a unique (p) in a set of size (c).
#define setp sprite->bv[0]
#define setc sprite->bv[1]

#define clock sprite->bv[3]
#define frame sprite->bv[4]
#define dx sprite->sv[0]
#define dy sprite->sv[1]
#define ttl sprite->sv[2]

#define FMN_SOULBALL_SPEED 4

/* Setup.
 */
 
static int8_t _soulball_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  if (!setc) return -1;
  
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  sprite->layer=100;
  sprite->image=&mainsprites;
  sprite->tileid=0x37;
  ttl=200;
  
  float t=(setp*M_PI*2.0f)/setc;
  dx=-sin(t)*FMN_SOULBALL_SPEED;
  dy=cos(t)*FMN_SOULBALL_SPEED;
  
  return 0;
}

/* Update.
 */
 
static void _soulball_update(struct fmn_sprite *sprite) {
  if (ttl--<=0) {
    fmn_sprite_del_later(sprite);
    return;
  }
  if (clock) clock--; else {
    clock=4;
    frame++;
    if (frame>=6) frame=0;
    switch (frame) {
      case 0: sprite->tileid=0x37; break;
      case 1: sprite->tileid=0x38; break;
      case 2: sprite->tileid=0x39; break;
      case 3: sprite->tileid=0x3a; break;
      case 4: sprite->tileid=0x39; break;
      case 5: sprite->tileid=0x38; break;
    }
  }
  sprite->x+=dx;
  sprite->y+=dy;
}

/* Type definition.
 */
 
const struct fmn_sprtype fmn_sprtype_soulball={
  .name="soulball",
  .init=_soulball_init,
  .update=_soulball_update,
  .render=fmn_sprite_render_default,
};
