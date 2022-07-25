#include "game/fullmoon.h"
#include "game/sprite/fmn_sprite.h"

const struct fmn_sprtype fmn_sprtype_dummy={
  .name="dummy",
  .render=fmn_sprite_render_default,
  .hitbox=fmn_sprite_hitbox_none,
};
