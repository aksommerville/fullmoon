#include "game/fullmoon.h"
#include "game/sprite/fmn_sprite.h"
#include "game/sprite/hero/fmn_hero.h"

#define statep sprite->bv[0]
#define offvalue sprite->bv[1]
#define onvalue sprite->bv[2]
#define basetileid sprite->bv[3]

static int8_t _fmn_treadle_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  basetileid=sprite->tileid;
  fmn_gstate[statep]=offvalue;
  return 0;
}

static uint8_t treadle_bestepped(struct fmn_sprite *sprite) {

  /* This would be nice and neat, but we also need to bestep by other solid sprites (eg pushbox)
  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  if (herox<sprite->x) return 0;
  if (heroy<sprite->y) return 0;
  if (herox>=sprite->x+sprite->w) return 0;
  if (heroy>=sprite->y+sprite->h) return 0;
  return 1;
  /**/
  
  //TODO Consider throttling this check if it gets too expensive. eg every 10th frame
  
  int16_t right=sprite->x+sprite->w;
  int16_t bottom=sprite->y+sprite->h;
  struct fmn_sprite **p=fmn_spritev;
  uint16_t i=fmn_spritec;
  for (;i-->0;p++) {
    struct fmn_sprite *stepper=*p;
    if (!(stepper->flags&FMN_SPRITE_FLAG_SOLID)) continue;
    int16_t midx=stepper->x+(stepper->w>>1);
    if (midx<sprite->x) continue;
    if (midx>=right) continue;
    int16_t midy=stepper->y+(stepper->h>>1);
    if (midy<sprite->y) continue;
    if (midy>=sprite->y+sprite->h) continue;
    return 1;
  }
  return 0;
}

static void _fmn_treadle_update(struct fmn_sprite *sprite) {
  if (treadle_bestepped(sprite)) {
    sprite->tileid=basetileid+1;
    fmn_gstate[statep]=1;
  } else {
    sprite->tileid=basetileid;
    fmn_gstate[statep]=0;
  }
}

const struct fmn_sprtype fmn_sprtype_treadle={
  .name="treadle",
  .init=_fmn_treadle_init,
  .update=_fmn_treadle_update,
  .render=fmn_sprite_render_default,
};
