#include "game/fullmoon.h"
#include "game/model/fmn_hero.h"
#include "game/sprite/fmn_sprite.h"

static void _heroproxy_del(struct fmn_sprite *sprite) {
}

static int8_t _heroproxy_init(struct fmn_sprite *sprite,const struct fmn_sprdef *sprdef) {
  return 0;
}

static void _heroproxy_update(struct fmn_sprite *sprite) {
// fmn_hero_update() gets called separately, by the top layer
  fmn_hero_get_outer_bounds(&sprite->x,&sprite->y,&sprite->w,&sprite->h);
}

static void _heroproxy_render(struct fmn_image *dst,int16_t scrollx,int16_t scrolly,struct fmn_sprite *sprite) {
  fmn_hero_render(dst);
}

static void _heroproxy_hitbox(int16_t *rx,int16_t *ry,int16_t *w,int16_t *h,struct fmn_sprite *sprite) {
  // Must match src/game/model/fmn_hero_public.c
  *rx=FMN_MM_PER_PIXEL;
  *ry=FMN_MM_PER_PIXEL;
  *w=FMN_MM_PER_TILE-(FMN_MM_PER_PIXEL*2);
  *h=FMN_MM_PER_TILE-(FMN_MM_PER_PIXEL*2);
}

const struct fmn_sprtype fmn_sprtype_heroproxy={
  .name="heroproxy",
  .del=_heroproxy_del,
  .init=_heroproxy_init,
  .update=_heroproxy_update,
  .render=_heroproxy_render,
  .hitbox=_heroproxy_hitbox,
};
