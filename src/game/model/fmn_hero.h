/* fmn_hero.h
 * The sprite the player controls.
 */
 
#ifndef FMN_HERO_H
#define FMN_HERO_H

struct fmn_hero {
  int16_t x,y; // mm in map space (not screen space). Top left of body tile.
  int16_t cellx,celly;
  int8_t dx,dy;
  uint8_t button;
  uint8_t walkspeed;
  uint8_t action;
  uint8_t bodyframe;
  uint8_t bodyanimtime;
  uint8_t facedir;
  uint8_t actionframe;
  uint8_t actionanimtime;
  int16_t actionparam;
  uint8_t spell[FMN_SPELL_LENGTH_LIMIT]; // FMN_DIR_(N,S,E,W)
  uint8_t spellc; // may exceed FMN_SPELL_LENGTH_LIMIT
  uint8_t spellrepudiation;
  uint8_t end_action_when_possible; // nonzero if an 'end action' is pending, eg flying over a hole
};

void fmn_hero_reset();
void fmn_hero_set_input(int8_t dx,int8_t dy,uint8_t button);
void fmn_hero_update();
void fmn_hero_render(struct fmn_image *fb);
void fmn_hero_get_world_position(int16_t *xmm,int16_t *ymm);
void fmn_hero_get_world_position_center(int16_t *xmm,int16_t *ymm);
void fmn_hero_get_outer_bounds(int16_t *xmm,int16_t *ymm,int16_t *w,int16_t *h);
void fmn_hero_get_screen_position(int16_t *xpx,int16_t *ypx);
uint8_t fmn_hero_set_action(uint8_t action); // => zero if rejected
uint8_t fmn_hero_get_action();
void fmn_hero_force_position(int16_t xmm,int16_t ymm);

#endif
