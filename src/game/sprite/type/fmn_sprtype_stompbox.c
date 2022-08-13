#include "game/fullmoon.h"
#include "game/sprite/fmn_sprite.h"
#include "game/sprite/hero/fmn_hero.h"

// stompbox is the same thing as treadle, but its state is sticky. Stepping on it toggles it.

#define statep sprite->bv[0]
#define offvalue sprite->bv[1]
#define onvalue sprite->bv[2]
#define basetileid sprite->bv[3]
#define pvstepped sprite->bv[4]

static int8_t _fmn_stompbox_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  basetileid=sprite->tileid;
  if (fmn_gstate[statep]==onvalue) sprite->tileid++;
  return 0;
}

static uint8_t stompbox_bestepped(struct fmn_sprite *sprite) {
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
    if ((stepper->type==&fmn_sprtype_hero)&&!fmn_hero_touching_ground()) continue;
    return 1;
  }
  return 0;
}

static void _fmn_stompbox_update(struct fmn_sprite *sprite) {
  if (stompbox_bestepped(sprite)) {
    if (pvstepped) return;
    pvstepped=1;
    if (fmn_gstate[statep]==onvalue) {
      sprite->tileid=basetileid;
      fmn_gstate[statep]=offvalue;
    } else {
      sprite->tileid=basetileid+1;
      fmn_gstate[statep]=onvalue;
    }
  } else {
    pvstepped=0;
  }
}

const struct fmn_sprtype fmn_sprtype_stompbox={
  .name="stompbox",
  .init=_fmn_stompbox_init,
  .update=_fmn_stompbox_update,
  .render=fmn_sprite_render_default,
};
