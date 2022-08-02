#include "game/sprite/hero/fmn_hero_internal.h"

/* Reset.
 */

void fmn_hero_reset() {
  fprintf(stderr,"%s\n",__func__);
  fmn_hero.facedir=FMN_DIR_S;
  fmn_hero.xfacedir=FMN_DIR_E;
  fmn_hero.spellrepudiation=0;
  //TODO
}

/* Receive input.
 */

void fmn_hero_set_input(int8_t dx,int8_t dy,uint8_t button) {
  if (!fmn_hero.sprite) return;
  uint8_t motionchanged=0;
  if (dx!=fmn_hero.indx) {
    fmn_hero.indx=dx;
    motionchanged=1;
  }
  if (dy!=fmn_hero.indy) {
    fmn_hero.indy=dy;
    motionchanged=1;
  }
  if (motionchanged) {
    fmn_hero_encode_motion();
    fmn_hero_update_facedir();
  }
  button=button?1:0;
  if (button!=fmn_hero.inbutton) {
    fmn_hero.inbutton=button;
    if (button) {
      fmn_hero_begin_action();
    } else {
      fmn_hero_end_action();
    }
  }
}

/* Trivial accessors.
 */

struct fmn_sprite *fmn_hero_get_sprite() {
  return fmn_hero.sprite;
}

void fmn_hero_get_world_position_center(int16_t *xmm,int16_t *ymm) {
  if (fmn_hero.sprite) {
    *xmm=fmn_hero.sprite->x+(fmn_hero.sprite->w>>1);
    *ymm=fmn_hero.sprite->y+(fmn_hero.sprite->h>>1);
  } else {
    *xmm=fmn_hero.holdposx;
    *ymm=fmn_hero.holdposy;
  }
}

void fmn_hero_adjust_position(int16_t dxmm,int16_t dymm) {
  if (fmn_hero.sprite) {
    fmn_hero.sprite->x+=dxmm;
    fmn_hero.sprite->y+=dymm;
    fmn_hero.cellx=(fmn_hero.sprite->x+(fmn_hero.sprite->w>>1))/FMN_MM_PER_TILE;
    fmn_hero.celly=(fmn_hero.sprite->y+(fmn_hero.sprite->h>>1))/FMN_MM_PER_TILE;
  } else {
    fmn_hero.holdposx+=dxmm;
    fmn_hero.holdposy+=dymm;
    fmn_hero.cellx=(fmn_hero.holdposx)/FMN_MM_PER_TILE;
    fmn_hero.celly=(fmn_hero.holdposy)/FMN_MM_PER_TILE;
  }
}

void fmn_hero_force_position(int16_t xmm,int16_t ymm) {
  if (fmn_hero.sprite) {
    fmn_hero.sprite->x=xmm-(fmn_hero.sprite->x>>1);
    fmn_hero.sprite->y=ymm-(fmn_hero.sprite->y>>1);
  } else {
    fmn_hero.holdposx=xmm;
    fmn_hero.holdposy=ymm;
  }
  fmn_hero.cellx=xmm/FMN_MM_PER_TILE;
  fmn_hero.celly=ymm/FMN_MM_PER_TILE;
}

void fmn_hero_set_action(uint8_t action) {
  fmn_hero.action=action;
}

uint8_t fmn_hero_get_action() {
  return fmn_hero.action;
}

/* Injure.
 */

void fmn_hero_injure(struct fmn_sprite *assailant) {
  fprintf(stderr,"%s %p\n",__func__,assailant);
  //TODO
}
