#include "game/sprite/hero/fmn_hero_internal.h"

/* Broom.
 */
 
static void fmn_hero_broom_begin() {
  fmn_hero.action_in_progress=FMN_ACTION_BROOM;
}

static void fmn_hero_broom_end() {
  fmn_hero.action_in_progress=0;
  //TODO must verify we're on solid ground, otherwise preserve action_in_progress
}

static void fmn_hero_broom_update() {
}

/* Feather.
 */
 
static void fmn_hero_feather_begin() {
  fmn_hero.action_in_progress=FMN_ACTION_FEATHER;
}

static void fmn_hero_feather_end() {
  fmn_hero.action_in_progress=0;
}

static void fmn_hero_feather_update() {
  //TODO Encode a feather command.
}

/* Wand.
 */
 
static void fmn_hero_wand_begin() {
  fmn_hero.action_in_progress=FMN_ACTION_WAND;
  fmn_hero.spellc=0;
  fmn_hero.spellrepudiation=0;
}

static void fmn_hero_wand_restart() {
  fmn_hero.spellc=0;
  fmn_hero.spellrepudiation=0;
}

static void fmn_hero_wand_repudiate() {
  fmn_hero.spellrepudiation=FMN_HERO_SPELL_REPUDIATION_TIME;
}

static void fmn_hero_wand_end() {
  if (!fmn_hero.spellc) {
    // Reject quietly, do nothing.
    fmn_hero.action_in_progress=0;
  } else if (fmn_hero.spellc>sizeof(fmn_hero.spellv)) {
    fmn_hero_wand_repudiate();
  } else if (!fmn_game_cast_spell(fmn_hero.spellv,fmn_hero.spellc)) {
    fmn_hero_wand_repudiate();
  } else {
    fmn_hero.action_in_progress=0;
  }
  fmn_hero.spellc=0;
}

static void fmn_hero_wand_encode(uint8_t dir) {

  // Attempting input during repudiation means she's done. Drop the action.
  if (fmn_hero.spellrepudiation) {
    fmn_hero.action_in_progress=0;
    fmn_hero.spellrepudiation=0;
    return;
  }

  if (fmn_hero.spellc<sizeof(fmn_hero.spellv)) {
    fmn_hero.spellv[fmn_hero.spellc]=dir;
  }
  // Let the count exceed storage. When we examine it later, we'll reject long ones blindly.
  if (fmn_hero.spellc<255) fmn_hero.spellc++;
}

static void fmn_hero_wand_motion() {
  if (!fmn_hero.indx) switch (fmn_hero.indy) {
    case -1: fmn_hero_wand_encode(FMN_DIR_N); break;
    case 1: fmn_hero_wand_encode(FMN_DIR_S); break;
  } else if (!fmn_hero.indy) switch (fmn_hero.indx) {
    case -1: fmn_hero_wand_encode(FMN_DIR_W); break;
    case 1: fmn_hero_wand_encode(FMN_DIR_E); break;
  }
}

static void fmn_hero_wand_update() {
  if (fmn_hero.spellrepudiation) {
    fmn_hero.spellrepudiation--;
    if (!fmn_hero.spellrepudiation) {
      fmn_hero.action_in_progress=0;
    }
  }
}

/* Umbrella.
 */
 
static void fmn_hero_umbrella_begin() {
  fmn_hero.action_in_progress=FMN_ACTION_UMBRELLA;
  fmn_hero.umbrellatime=0;
}

static void fmn_hero_umbrella_end() {
  fmn_hero.action_in_progress=0;
  fmn_hero_update_facedir();
}

static void fmn_hero_umbrella_update() {
  if (fmn_hero.umbrellatime<FMN_HERO_UMBRELLA_TIME) {
    fmn_hero.umbrellatime++;
    if (fmn_hero.umbrellatime==FMN_HERO_UMBRELLA_TIME) {
      // sound effect? umbrella just popped open.
    }
  } else {
    //TODO deflect projectiles. does that happen here?
  }
}

/* Digested input events.
 */
 
void fmn_hero_begin_action() {
  if (fmn_hero.action_in_progress==fmn_hero.action) {
    // Restarting the existing action. (also "no action", all good).
    // This is usually either impossible or meaningless, but it does come up with the wand, when repudiating a spell.
    switch (fmn_hero.action) {
      case FMN_ACTION_WAND: fmn_hero_wand_restart(); break;
    }
    return;
  }
  if (fmn_hero.action_in_progress) {
    // Reject new action because one is still in progress.
    // eg flying over a hole, you release A but the action continues until you reach solid ground.
    // You can select a new action in that state, but you can't use it yet.
    return;
  }
  switch (fmn_hero.action) {
    case FMN_ACTION_BROOM: fmn_hero_broom_begin(); break;
    case FMN_ACTION_FEATHER: fmn_hero_feather_begin(); break;
    case FMN_ACTION_WAND: fmn_hero_wand_begin(); break;
    case FMN_ACTION_UMBRELLA: fmn_hero_umbrella_begin(); break;
  }
}
 
void fmn_hero_end_action() {
  switch (fmn_hero.action_in_progress) {
    case 0: break;
    case FMN_ACTION_BROOM: fmn_hero_broom_end(); break;
    case FMN_ACTION_FEATHER: fmn_hero_feather_end(); break;
    case FMN_ACTION_WAND: fmn_hero_wand_end(); break;
    case FMN_ACTION_UMBRELLA: fmn_hero_umbrella_end(); break;
    default: fmn_hero.action_in_progress=0; break; // oops?
  }
}

void fmn_hero_encode_motion() {
  switch (fmn_hero.action_in_progress) {
    case FMN_ACTION_BROOM: break;
    case FMN_ACTION_FEATHER: break;
    case FMN_ACTION_WAND: fmn_hero_wand_motion(); break;
    case FMN_ACTION_UMBRELLA: break;
  }
}

void fmn_hero_update_action() {
  switch (fmn_hero.action_in_progress) {
    case FMN_ACTION_BROOM: fmn_hero_broom_update(); break;
    case FMN_ACTION_FEATHER: fmn_hero_feather_update(); break;
    case FMN_ACTION_WAND: fmn_hero_wand_update(); break;
    case FMN_ACTION_UMBRELLA: fmn_hero_umbrella_update(); break;
  }
}
