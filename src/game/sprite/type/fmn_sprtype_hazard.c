#include "game/fullmoon.h"
#include "game/sprite/fmn_sprite.h"
#include "game/sprite/hero/fmn_hero.h"

//TODO I'd prefer (animtilec,animframelength) to be in the sprdef. For now, we can only hard-code or specify in POI.
#define animtilec sprite->bv[0]
#define animframelength sprite->bv[1]
#define animframe sprite->bv[4]
#define animclock sprite->bv[5]
#define basetileid sprite->bv[6]

static int8_t _fmn_hazard_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  basetileid=sprite->tileid;
  return 0;
}

static void _fmn_hazard_update(struct fmn_sprite *sprite) {

  if (animtilec&&animframelength) {
    animclock++;
    if (animclock>=animframelength) {
      animclock=0;
      animframe++;
      if (animframe>=animtilec) {
        animframe=0;
      }
      sprite->tileid=basetileid+animframe;
    }
  }

  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  if (herox<sprite->x) return;
  if (heroy<sprite->y) return;
  if (herox>=sprite->x+sprite->w) return;
  if (heroy>=sprite->y+sprite->h) return;
  fmn_hero_injure(sprite);
}

const struct fmn_sprtype fmn_sprtype_hazard={
  .name="hazard",
  .init=_fmn_hazard_init,
  .update=_fmn_hazard_update,
  .render=fmn_sprite_render_default,
};
