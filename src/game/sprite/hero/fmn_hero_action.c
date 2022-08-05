#include "game/sprite/hero/fmn_hero_internal.h"

/* Broom.
 */
 
static void fmn_hero_broom_begin() {
  fmn_hero.action_in_progress=FMN_ACTION_BROOM;
}

static void fmn_hero_broom_end() {
  if (fmn_sprite_collide(0,0,fmn_hero.sprite,FMN_TILE_SOLID|FMN_TILE_HOLE,0,0)) {
    // We must be over water -- don't get off the broom.
    return;
  }
  fmn_hero.action_in_progress=0;
}

static void fmn_hero_broom_update() {
  if (!fmn_hero.inbutton) {
    // User tried to get off the broom and we declined.
    // Check again every frame.
    if (!fmn_sprite_collide(0,0,fmn_hero.sprite,FMN_TILE_SOLID|FMN_TILE_HOLE,0,0)) {
      fmn_hero.action_in_progress=0;
    }
  }
}

/* Feather.
 */
 
static struct fmn_sprite *fmn_hero_feather_find_target() {
  if (!fmn_hero.sprite) return 0;
  int16_t x=fmn_hero.sprite->x+(fmn_hero.sprite->w>>1);
  int16_t y=fmn_hero.sprite->y+(fmn_hero.sprite->h>>1);
  switch (fmn_hero.facedir) {
    case FMN_DIR_N: y-=(fmn_hero.sprite->h*5)/3; break;
    case FMN_DIR_S: y+=(fmn_hero.sprite->h*4)/3; break;
    case FMN_DIR_W: x-=(fmn_hero.sprite->w*4)/3; break;
    case FMN_DIR_E: x+=(fmn_hero.sprite->w*4)/3; break;
  }
  struct fmn_sprite **p=fmn_spritev;
  uint16_t i=fmn_spritec;
  for (;i-->0;p++) {
    struct fmn_sprite *target=*p;
    if (!target->type->featherspell) continue;
    if (target->x>x) continue;
    if (target->y>y) continue;
    if (target->x+target->w<=x) continue;
    if (target->y+target->h<=y) continue;
    return target;
  }
  return 0;
}

static void fmn_hero_feather_check(uint8_t force) {

  struct fmn_sprite *target=fmn_hero_feather_find_target();
  if (!target) {
    // If we leave the target, nix (featherdir) so we can return to it and encode another command.
    fmn_hero.featherdir=0;
    return;
  }
  
  uint8_t dir;
  switch (fmn_hero.facedir) {
    case FMN_DIR_N: dir=FMN_DIR_S; break;
    case FMN_DIR_S: dir=FMN_DIR_N; break;
    case FMN_DIR_W: dir=FMN_DIR_E; break;
    case FMN_DIR_E: dir=FMN_DIR_W; break;
    default: return;
  }
  
  // Without (force), ignore duplicate commands.
  if (!force) {
    if ((target==fmn_hero.feathertarget)&&(dir==fmn_hero.featherdir)) return;
  }
  fmn_hero.featherdir=dir;
  if (target!=fmn_hero.feathertarget) {
    fmn_hero.feathertarget=target;
    fmn_hero.featherspellc=0;
  }
  
  // Add to the queue, possibly evicting a command from its head.
  if (fmn_hero.featherspellc>=sizeof(fmn_hero.featherspellv)) {
    fmn_hero.featherspellc=sizeof(fmn_hero.featherspellv)-1;
    memmove(fmn_hero.featherspellv,fmn_hero.featherspellv+1,fmn_hero.featherspellc);
  }
  fmn_hero.featherspellv[fmn_hero.featherspellc++]=dir;
  if (target->type->featherspell(target,fmn_hero.featherspellv,fmn_hero.featherspellc)) {
    fmn_hero.featherspellc=0;
  }
}
 
static void fmn_hero_feather_begin() {
  fmn_hero.action_in_progress=FMN_ACTION_FEATHER;
  if (fmn_hero.framec>=fmn_hero.featherofftime+FMN_HERO_FEATHER_FORGET_TIME) {
    fmn_hero.feathertarget=0;
    fmn_hero.featherspellc=0;
    fmn_hero.featherdir=0;
  }
  fmn_hero_feather_check(1);
}

static void fmn_hero_feather_end() {
  fmn_hero.action_in_progress=0;
  fmn_hero.featherofftime=fmn_hero.framec;
}

static void fmn_hero_feather_update() {
  if (fmn_hero.indx||fmn_hero.indy) {
    fmn_hero_feather_check(0);
  }
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

/* Pushing things.
 * This triggers just once per "push session", when she has been trying but failing to move for 30 frames.
 */
 
void fmn_hero_check_push() {
  
  /* Cardinal directions only.
   * If she's blocked on a diagonal, there must be two other parties involved, and this is a one-at-a-time proposition.
   * The direction comes from (indx,indy) not (facedir) -- she might have the umbrella deployed or something, we care only about the motion.
   */
  if (fmn_hero.indx&&fmn_hero.indy) return;
  if (!fmn_hero.indx&&!fmn_hero.indy) return;
  int16_t ckx=fmn_hero.sprite->x+(fmn_hero.sprite->w>>1)+fmn_hero.sprite->w*fmn_hero.indx;
  int16_t cky=fmn_hero.sprite->y+(fmn_hero.sprite->h>>1)+fmn_hero.sprite->h*fmn_hero.indy;
  
  /* Stop after we find one.
   * Pushables must be solid and we're checking just one point, so there can only be one.
   */
  struct fmn_sprite **p=fmn_spritev;
  uint16_t i=fmn_spritec;
  for (;i-->0;p++) {
    struct fmn_sprite *pumpkin=*p;
    if (!pumpkin->type->push) continue;
    if (!(pumpkin->flags&FMN_SPRITE_FLAG_SOLID)) continue;
    if (pumpkin->x>ckx) continue;
    if (pumpkin->y>cky) continue;
    if (pumpkin->x+pumpkin->w<=ckx) continue;
    if (pumpkin->y+pumpkin->h<=cky) continue;
    pumpkin->type->push(pumpkin,fmn_hero.indx,fmn_hero.indy);
    return;
  }
}
