#include "game/fullmoon.h"
#include "game/sprite/fmn_sprite.h"

#define statep sprite->bv[0]
#define openvalue sprite->bv[1]
#define closedvalue sprite->bv[2]
#define basetileid sprite->bv[3]

static int8_t _fmn_blockade_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  basetileid=sprite->tileid;
  return 0;
}

static void _fmn_blockade_update(struct fmn_sprite *sprite) {
  if (fmn_gstate[statep]==closedvalue) {
    sprite->tileid=basetileid;
    sprite->flags|=FMN_SPRITE_FLAG_SOLID;
  } else {
    sprite->tileid=basetileid+1;
    sprite->flags&=~FMN_SPRITE_FLAG_SOLID;
  }
}

const struct fmn_sprtype fmn_sprtype_blockade={
  .name="blockade",
  .init=_fmn_blockade_init,
  .update=_fmn_blockade_update,
  .render=fmn_sprite_render_default,
};
