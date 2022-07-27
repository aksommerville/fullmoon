#include "game/fullmoon.h"
#include "game/sprite/fmn_sprite.h"

static int8_t _fmn_dummy_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  return 0;
}

const struct fmn_sprtype fmn_sprtype_dummy={
  .name="dummy",
  .init=_fmn_dummy_init,
  .render=fmn_sprite_render_default,
  .hitbox=fmn_sprite_hitbox_all,
};
