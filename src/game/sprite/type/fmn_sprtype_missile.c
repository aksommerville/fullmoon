#include "game/fullmoon.h"
#include "game/sprite/hero/fmn_hero.h"
#include "game/sprite/fmn_sprite.h"

#define ttl sprite->sv[0]
#define dx sprite->sv[1]
#define dy sprite->sv[2]

#define FMN_MISSILE_DEFAULT_TTL 160

static int8_t _fmn_missile_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  ttl=FMN_MISSILE_DEFAULT_TTL;
  return 0;
}

static void _fmn_missile_update(struct fmn_sprite *sprite) {
  if (ttl>0) ttl--;
  else {
    fmn_sprite_del_later(sprite);
    return;
  }
  sprite->x+=dx;
  sprite->y+=dy;
  
  // Non-lethal if not moving (eg raccoon brandishing me).
  if (!dx&&!dy) return;
  
  int16_t herox,heroy,herow,heroh;
  fmn_hero_get_world_bounds(&herox,&heroy,&herow,&heroh);
  if (sprite->x<herox) return;
  if (sprite->y<heroy) return;
  if (sprite->x>=herox+herow) return;
  if (sprite->y>=heroy+heroh) return;
  fmn_sprite_del_later(sprite);
  fmn_hero_injure(sprite);
}

//TODO blink when ttl near zero

const struct fmn_sprtype fmn_sprtype_missile={
  .name="missile",
  .init=_fmn_missile_init,
  .update=_fmn_missile_update,
  .render=fmn_sprite_render_default,
};
