#include "game/sprite/hero/fmn_hero_internal.h"
#include "game/model/fmn_map.h"

/* Position changed, after collisions resolved.
 * Check scrolling, treadles, etc.
 */
 
static void fmn_hero_examine_new_position() {

  int16_t x=fmn_hero.sprite->x+(fmn_hero.sprite->w>>1);
  int16_t y=fmn_hero.sprite->y+(fmn_hero.sprite->h>>1);
  int16_t cellx=x/FMN_MM_PER_TILE; if (x<0) cellx--;
  int16_t celly=y/FMN_MM_PER_TILE; if (y<0) celly--;
  if ((cellx==fmn_hero.cellx)&&(celly==fmn_hero.celly)) return;
  
  // If we are flying, keep (cellx,celly) up to date but don't trigger anything.
  if (fmn_hero.action_in_progress==FMN_ACTION_BROOM) {
    fmn_hero.cellx=cellx;
    fmn_hero.celly=celly;
    return;
  }
  
  // Changed cell, normal case.
  if (!(fmn_hero.cellx&0xff00)&&!(fmn_hero.celly&0xff00)) {
    fmn_map_exit_cell(fmn_hero.cellx,fmn_hero.celly);
  }
  fmn_hero.cellx=cellx;
  fmn_hero.celly=celly;
  if (!(fmn_hero.cellx&0xff00)&&!(fmn_hero.celly&0xff00)) {
    fmn_map_enter_cell(fmn_hero.cellx,fmn_hero.celly);
  }
}

/* Motion.
 */
 
static void fmn_hero_update_motion() {

  // Motion stops cold when you use the wand.
  if (fmn_hero.action_in_progress==FMN_ACTION_WAND) {
    fmn_hero.walkspeed=0;
    fmn_hero.pushc=0;
    return;
  }

  // Apply motion optimistically and check for collisions.
  int16_t dx=fmn_hero.indx*fmn_hero.walkspeed;
  int16_t dy=fmn_hero.indy*fmn_hero.walkspeed;
  if (dx||dy) {
    int16_t pvx=fmn_hero.sprite->x;
    int16_t pvy=fmn_hero.sprite->y;
    fmn_hero.sprite->x+=dx;
    fmn_hero.sprite->y+=dy;
    int16_t adjx,adjy;
    uint8_t cellprops=
      FMN_TILE_SOLID|
      ((fmn_hero.action_in_progress==FMN_ACTION_BROOM)?0:FMN_TILE_HOLE)|
    0;
    if (fmn_sprite_collide(&adjx,&adjy,fmn_hero.sprite,cellprops,FMN_SPRITE_FLAG_SOLID,1)) {
      if (!adjx&&!adjy) {
        // Unsolvable collision! Put her back where we found her.
        // Alternately, we could just let it ride, she'll walk through walls for a while and snap back when she leaves it.
        fmn_hero.sprite->x-=dx;
        fmn_hero.sprite->y-=dy;
      }
    }
    dx=fmn_hero.sprite->x-pvx;
    dy=fmn_hero.sprite->y-pvy;
    if (!dx&&!dy) {
      if (fmn_hero.pushc<0xffff) fmn_hero.pushc++;
      if (fmn_hero.pushc==FMN_HERO_PUSH_DELAY) fmn_hero_check_push();
    } else {
      fmn_hero.pushc=0;
    }
  } else {
    fmn_hero.pushc=0;
  }
  
  // Advance walkspeed toward its target.
  int16_t targetspeed;
  if (!fmn_hero.indx&&!fmn_hero.indy) targetspeed=0;
  else if (fmn_hero.action_in_progress==FMN_ACTION_BROOM) targetspeed=FMN_HERO_FLY_SPEED;
  else targetspeed=FMN_HERO_WALK_SPEED;
  if (fmn_hero.walkspeed<targetspeed) fmn_hero.walkspeed++;
  else if (fmn_hero.walkspeed>targetspeed) fmn_hero.walkspeed--;
  
  // React to a change of position.
  if (dx||dy) fmn_hero_examine_new_position();
}

/* Update, main entry point.
 */
 
void fmn_hero_update(struct fmn_sprite *sprite) {
  /* There can be one update cycle, where the sprite has not been deleted but we have already dropped it.
   * That's fine, just get out.
   */
  if (!fmn_hero.sprite) return;
  fmn_hero_update_motion();
  if (fmn_hero.action_in_progress) fmn_hero_update_action();
}
