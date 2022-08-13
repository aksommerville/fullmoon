/* fmn_hero.h
 * Public interface to the hero sprite.
 */
 
#ifndef FMN_HERO_H
#define FMN_HERO_H

void fmn_hero_reset();
void fmn_hero_set_input(int8_t dx,int8_t dy,uint8_t button);
void fmn_hero_get_world_position_center(int16_t *xmm,int16_t *ymm);
void fmn_hero_get_world_bounds(int16_t *xmm,int16_t *ymm,int16_t *wmm,int16_t *hmm);
void fmn_hero_set_action(uint8_t action);
uint8_t fmn_hero_get_action();
void fmn_hero_injure(struct fmn_sprite *assailant);
struct fmn_sprite *fmn_hero_get_sprite();
uint8_t fmn_hero_get_facedir();
uint8_t fmn_hero_touching_ground();

// Zero if feather not being used, otherwise a cardinal direction.
uint8_t fmn_hero_get_feather_dir();

/* Umbrella (or possibly some future item I haven't thought of yet).
 * If deployed, returns the direction of deployment and depth.
 * (x_or_y) returns with the absolute position of the shield's interior edge. The other three edges match the hero's own bounds.
 * Any missile approaching that edge should deflect.
 * Returns zero if not deployed.
 */
uint8_t fmn_hero_get_deflector(int16_t *x_or_y);

/* For doors and such, change the position but don't treat it as diegetic movement.
 * We update our own cell-tracking state such that we will *not* reporting having stepped on this cell.
 */
void fmn_hero_adjust_position(int16_t dxmm,int16_t dymm);
void fmn_hero_force_position(int16_t xmm,int16_t ymm);

#endif
