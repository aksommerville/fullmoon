#include "game/fullmoon.h"
#include "game/model/fmn_map.h"
#include "game/sprite/fmn_sprite.h"
#include <stdlib.h>
#include <stdio.h>

static int8_t _pushbox_init(struct fmn_sprite *sprite,const struct fmn_sprdef *sprdef) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  return 0;
}

static void _pushbox_update(struct fmn_sprite *sprite) {
}

static void _pushbox_collide(struct fmn_sprite *sprite,struct fmn_sprite *other) {
  fprintf(stderr,"%s %p[%s]\n",__func__,other,other?other->type->name:"");
}

const struct fmn_sprtype fmn_sprtype_pushbox={
  .name="pushbox",
  .init=_pushbox_init,
  .update=_pushbox_update,
  .render=fmn_sprite_render_default,
  .hitbox=fmn_sprite_hitbox_all,
  .collide=_pushbox_collide,
};
