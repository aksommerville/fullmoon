#include "game/sprite/hero/fmn_hero_internal.h"

struct fmn_hero fmn_hero={0};

static void _hero_del(struct fmn_sprite *sprite) {
  if (sprite==fmn_hero.sprite) {
    fmn_hero.holdposx=sprite->x+(sprite->w>>1);
    fmn_hero.holdposy=sprite->y+(sprite->h>>1);
    fmn_hero.sprite=0;
    fmn_hero.feathertarget=0;
    fmn_hero.featherspellc=0;
    fmn_hero.spellc=0;
  }
}

static int8_t _hero_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  
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
  
  fmn_hero.feathertarget=0;
  fmn_hero.featherspellc=0;
  fmn_hero.spellc=0;
    
  return 0;
}

/* Type definition.
 */
 
const struct fmn_sprtype fmn_sprtype_hero={
  .name="hero",
  .del=_hero_del,
  .init=_hero_init,
  .update=fmn_hero_update,
  .render=fmn_hero_render,
};
