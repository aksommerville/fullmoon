#include "game/sprite/hero/fmn_hero_internal.h"
#include "game/ui/fmn_play.h"

/* Reset.
 */

void fmn_hero_reset() {
  fmn_hero.facedir=FMN_DIR_S;
  fmn_hero.xfacedir=FMN_DIR_E;
  fmn_hero.spellrepudiation=0;
  fmn_hero.feathertarget=0;
  fmn_hero.featherspellc=0;
  fmn_hero.spellc=0;
  fmn_hero.walkspeed=0;
  fmn_hero.action_in_progress=0;
  fmn_hero.indx=0;
  fmn_hero.indy=0;
  fmn_hero.inbutton=0;
  fmn_hero.form=FMN_HERO_FORM_NORMAL;
}

/* Receive input.
 */

void fmn_hero_set_input(int8_t dx,int8_t dy,uint8_t button) {
  if (!fmn_hero.sprite) return;
  uint8_t motionchanged=0;
  if (dy!=fmn_hero.indy) {
    switch (dy) {
      case -1: fmn_hero.last_motion_dir=FMN_DIR_N; break;
      case 1: fmn_hero.last_motion_dir=FMN_DIR_S; break;
    }
    fmn_hero.indy=dy;
    motionchanged=1;
  }
  if (dx!=fmn_hero.indx) {
    switch (dx) {
      case -1: fmn_hero.last_motion_dir=FMN_DIR_W; break;
      case 1: fmn_hero.last_motion_dir=FMN_DIR_E; break;
    }
    fmn_hero.indx=dx;
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

void fmn_hero_get_world_bounds(int16_t *xmm,int16_t *ymm,int16_t *wmm,int16_t *hmm) {
  if (fmn_hero.sprite) {
    *xmm=fmn_hero.sprite->x+FMN_HERO_BOUNDS_LEFT;
    *ymm=fmn_hero.sprite->y+FMN_HERO_BOUNDS_UP;
    *wmm=fmn_hero.sprite->w-FMN_HERO_BOUNDS_LEFT+FMN_HERO_BOUNDS_RIGHT;
    *hmm=fmn_hero.sprite->h-FMN_HERO_BOUNDS_UP+FMN_HERO_BOUNDS_DOWN;
  } else {
    *xmm=fmn_hero.holdposx;
    *ymm=fmn_hero.holdposy;
    *wmm=FMN_MM_PER_TILE;
    *hmm=FMN_MM_PER_TILE;
  }
  // Add umbrella depth if deployed.
  if (fmn_hero.action_in_progress==FMN_ACTION_UMBRELLA) switch (fmn_hero.facedir) {
    case FMN_DIR_N: (*ymm)-=FMN_HERO_UMBRELLA_DEPTH; (*hmm)+=FMN_HERO_UMBRELLA_DEPTH; break;
    case FMN_DIR_W: (*xmm)-=FMN_HERO_UMBRELLA_DEPTH; (*wmm)+=FMN_HERO_UMBRELLA_DEPTH; break;
    case FMN_DIR_S: (*hmm)+=FMN_HERO_UMBRELLA_DEPTH; break;
    case FMN_DIR_E: (*wmm)+=FMN_HERO_UMBRELLA_DEPTH; break;
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
    fmn_hero.sprite->x=xmm-(fmn_hero.sprite->w>>1);
    fmn_hero.sprite->y=ymm-(fmn_hero.sprite->h>>1);
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

uint8_t fmn_hero_get_facedir() {
  return fmn_hero.facedir;
}

uint8_t fmn_hero_get_feather_dir() {
  if (fmn_hero.action_in_progress==FMN_ACTION_FEATHER) return fmn_hero.facedir;
  return 0;
}

uint8_t fmn_hero_touching_ground() {
  if (fmn_hero.action_in_progress==FMN_ACTION_BROOM) return 0;
  return 1;
}

uint8_t fmn_hero_get_form() {
  return fmn_hero.form;
}

/* Deployed umbrella.
 */
 
uint8_t fmn_hero_get_deflector(int16_t *x_or_y) {
  if (fmn_hero.action_in_progress!=FMN_ACTION_UMBRELLA) return 0;
  if (fmn_hero.umbrellatime<FMN_HERO_UMBRELLA_TIME) return 0; // not deployed yet
  if (!fmn_hero.sprite) return 0;
  switch (fmn_hero.facedir) {
    case FMN_DIR_N: *x_or_y=fmn_hero.sprite->y+FMN_HERO_UMBRELLA_DEPTH; break;
    case FMN_DIR_W: *x_or_y=fmn_hero.sprite->x+FMN_HERO_UMBRELLA_DEPTH; break;
    case FMN_DIR_S: *x_or_y=fmn_hero.sprite->y+fmn_hero.sprite->h-FMN_HERO_UMBRELLA_DEPTH; break;
    case FMN_DIR_E: *x_or_y=fmn_hero.sprite->y+fmn_hero.sprite->w-FMN_HERO_UMBRELLA_DEPTH; break;
  }
  return fmn_hero.facedir;
}

/* Injure.
 */

void fmn_hero_injure(struct fmn_sprite *assailant) {
  int16_t x=0,y=0;
  if (fmn_hero.sprite) {
    x=fmn_hero.sprite->x+(fmn_hero.sprite->w>>1);
    y=fmn_hero.sprite->y+(fmn_hero.sprite->h>>1);
    fmn_sprite_del_later(fmn_hero.sprite);
    fmn_hero.sprite=0;
  }
  fmn_game_end(x,y);
}

/* Transform.
 */
 
void fmn_hero_set_form(uint8_t form) {
  if (form==fmn_hero.form) return;
  fmn_hero_end_action();
  if (fmn_hero.action_in_progress) return;
  fmn_hero.form=form;
  int16_t x=fmn_hero.sprite->x+(fmn_hero.sprite->w>>1);
  int16_t y=fmn_hero.sprite->y+(fmn_hero.sprite->h>>1);
  uint8_t i=7; while (i-->0) {
    struct fmn_sprite *sprite=fmn_sprite_new(&fmn_sprtype_soulball,0,x,y,i,7,0);
  }
}

void fmn_hero_become_pumpkin() {
  fmn_hero_set_form(FMN_HERO_FORM_PUMPKIN);
}
