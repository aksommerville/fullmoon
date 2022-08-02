/* fmn_hero_internal.h
 * The hero is a sprite, but it also has a bunch of global state outside the sprite structure.
 */

#ifndef FMN_HERO_INTERNAL_H
#define FMN_HERO_INTERNAL_H

#include "../../fullmoon.h"
#include "../fmn_sprite.h"
#include "fmn_hero.h"
#include <stdio.h>

#define FMN_HERO_WALK_SPEED 6
#define FMN_HERO_FLY_SPEED 9
#define FMN_HERO_SPELL_REPUDIATION_TIME 60
#define FMN_HERO_UMBRELLA_TIME 20

extern struct fmn_hero {

  struct fmn_sprite *sprite;
  
  int8_t indx,indy;
  uint8_t inbutton;
  
  int16_t walkspeed;
  int16_t cellx,celly; // Use int16_t because they can go OOB.
  int16_t holdposx,holdposy; // Keep position here when the sprite goes out of scope.
  uint8_t facedir; // FMN_DIR_(N,S,E,W)
  uint8_t xfacedir; // FMN_DIR_(E,W)
  uint16_t framec; // Frame count since init, for general animation.
  uint8_t walkframe;
  uint8_t walkframeclock;
  
  uint8_t action;
  uint8_t action_in_progress; // action ID if in progress. (not necessarily matching (action) or (button)).
  
  uint8_t spellv[8]; // Longest spell is 8 motions. We can change this arbitrarily, up to 254.
  uint8_t spellc;
  uint8_t spellrepudiation;
  
  uint8_t umbrellatime; // Counts up to FMN_HERO_UMBRELLA_TIME while deploying (closed).
  
} fmn_hero;

void fmn_hero_update_facedir();
void fmn_hero_render(struct fmn_image *dst,int16_t scrollx,int16_t scrolly,struct fmn_sprite *sprite);

void fmn_hero_update(struct fmn_sprite *sprite);

// Called immediately after receiving button events.
// (inbutton) is already updated. These will update (action_in_progress) etc.
void fmn_hero_begin_action();
void fmn_hero_end_action();
void fmn_hero_update_action();

// Call whenever (indx,indy) changes, updates wand encoder etc.
void fmn_hero_encode_motion();

#endif
