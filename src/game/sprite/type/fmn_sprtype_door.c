#include "game/fullmoon.h"
#include "game/sprite/hero/fmn_hero.h"
#include "game/sprite/fmn_sprite.h"

// Doors are just dummy with SOLID and OPENABLE.
// But there's a gotcha -- they'll be closed at init, and the hero might be initting on this very spot. (doors are like that)
// So we have an update hook that checks for hero overlap and deletes self if so.
// ugly but whatever

static int8_t _fmn_door_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  return 0;
}

static void _fmn_door_update(struct fmn_sprite *sprite) {
  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  if (herox<sprite->x) return;
  if (heroy<sprite->y) return;
  if (herox>=sprite->x+sprite->w) return;
  if (heroy>=sprite->y+sprite->h) return;
  // hero ended up colliding with us -- he must have travelled into this door from elsewhere. we ought to be open
  fmn_sprite_del_later(sprite);
}

const struct fmn_sprtype fmn_sprtype_door={
  .name="dummy",
  .init=_fmn_door_init,
  .update=_fmn_door_update,
  .render=fmn_sprite_render_default,
};
