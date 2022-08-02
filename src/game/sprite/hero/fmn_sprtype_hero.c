#include "game/sprite/hero/fmn_hero_internal.h"

struct fmn_hero fmn_hero={0};

static void _hero_del(struct fmn_sprite *sprite) {
  fprintf(stderr,"%s\n",__func__);
  if (sprite==fmn_hero.sprite) {
    fmn_hero.holdposx=sprite->x+(sprite->w>>1);
    fmn_hero.holdposy=sprite->y+(sprite->h>>1);
    fmn_hero.sprite=0;
  }
}

static int8_t _hero_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  fprintf(stderr,"%s %p\n",__func__,sprite);
  
  if (fmn_hero.sprite) {
    fprintf(stderr,"%s: Multiple hero sprites\n",__func__);
    return -1;
  }
  fmn_hero.sprite=sprite;
  sprite->x=fmn_hero.holdposx;
  sprite->y=fmn_hero.holdposy;
  
  sprite->flags=FMN_SPRITE_FLAG_SOLID;
  sprite->w=(FMN_MM_PER_TILE*6)/8;
  sprite->h=(FMN_MM_PER_TILE*6)/8;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  return 0;
}

static void _hero_collide(struct fmn_sprite *sprite,struct fmn_sprite *other) {
  fprintf(stderr,"%s %p\n",__func__,other);
  //TODO
}

static void _hero_hitbox(int16_t *rx,int16_t *ry,int16_t *w,int16_t *h,struct fmn_sprite *sprite) {
  fprintf(stderr,"%s\n",__func__);
  //TODO
}

/* Type definition.
 */
 
const struct fmn_sprtype fmn_sprtype_hero={
  .name="hero",
  .del=_hero_del,
  .init=_hero_init,
  .update=fmn_hero_update,
  .collide=_hero_collide,
  .render=fmn_hero_render,
  .hitbox=_hero_hitbox,
};
