#include "game/fullmoon.h"
#include "game/ui/fmn_play.h"
#include "game/model/fmn_map.h"
#include "game/sprite/hero/fmn_hero.h"
#include "game/sprite/fmn_sprite.h"
#include <math.h>

#define animclock sprite->bv[0]
#define frame sprite->bv[1]
#define basetileid sprite->bv[3]
#define phase sprite->bv[4]
#define walkdir sprite->bv[5]
#define menacecount sprite->bv[6]
#define charmed sprite->bv[7]
#define xlimit sprite->sv[0] /* left edge of my screen, in map mm */
#define ylimit sprite->sv[1]
#define phasetime sprite->sv[2]
#define missile sprite->pv[0]
#define MISSILE ((struct fmn_sprite*)sprite->pv[0])

#define FMN_RACCOON_PHASE_IDLE 0
#define FMN_RACCOON_PHASE_WALK 1
#define FMN_RACCOON_PHASE_MENACE 2

/* Bear with me here:
 * WALK_TIME*WALK_SPEED should be a multiple of FMN_MM_PER_TILE so he tends to stay on cell boundaries.
 * WALK_TIME+IDLE_TIME should be prime to randomize the clock when we pick a direction.
 * 128*2+55=311, which is prime
 * 96*2+71=263, which is prime
 */
#define FMN_RACCOON_IDLE_TIME 71
#define FMN_RACCOON_WALK_TIME 96
#define FMN_RACCOON_WALK_SPEED 2

// ...ok actually that's not good enough, because the walk time can be cut short, and he can end up in phase with the clock.
// Randomize the idle time a little by adding some low bits from the clock.
#define FMN_RACCOON_IDLE_TIME_EXTRA 0x1f

#define FMN_RACCOON_MENACE_TIME 30
#define FMN_RACCOON_MENACE_PERIOD 4 /* after so many idle/walk cycles, throw something */
#define FMN_RACCOON_TOSS_SPEED 6 /* mm/frame, fractional ok */

static void fmn_raccoon_generate_missile(struct fmn_sprite *sprite);
static uint8_t fmn_raccoon_can_throw(struct fmn_sprite *sprite);

/* Setup.
 */
 
static void fmn_raccoon_measure_limits(struct fmn_sprite *sprite) {
  xlimit=(sprite->x/FMN_SCREENW_MM)*FMN_SCREENW_MM;
  ylimit=(sprite->y/FMN_SCREENH_MM)*FMN_SCREENH_MM;
}

static int8_t _fmn_raccoon_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  basetileid=sprite->tileid;
  fmn_raccoon_measure_limits(sprite);
  phase=FMN_RACCOON_PHASE_IDLE;
  phasetime=FMN_RACCOON_IDLE_TIME;
  menacecount=fmn_play_frame_count%FMN_RACCOON_MENACE_PERIOD;
  return 0;
}

/* Idle.
 */

static void fmn_raccoon_idle_end(struct fmn_sprite *sprite) {
  
  // Sometimes throw a thing instead of walking.
  if (fmn_raccoon_can_throw(sprite)) {
    menacecount++;
    if (menacecount>=FMN_RACCOON_MENACE_PERIOD) {
      menacecount=0;
      phase=FMN_RACCOON_PHASE_MENACE;
      phasetime=FMN_RACCOON_MENACE_TIME;
      fmn_raccoon_generate_missile(sprite);
      return;
    }
  }
  
  phase=FMN_RACCOON_PHASE_WALK;
  phasetime=FMN_RACCOON_WALK_TIME;
  
  // Measure freedom in each cardinal direction, and reduce to the screen edges.
  uint8_t freedom[4];
  fmn_sprite_measure_cardinal_freedom(freedom,sprite,FMN_TILE_SOLID|FMN_TILE_HOLE,FMN_SPRITE_FLAG_SOLID);
  fmn_sprite_limit_freedom_to_screen(sprite,freedom);
  
  // List free directions.
  struct fmn_raccoon_walk_option {
    uint8_t freedom;
    uint8_t dir;
  } optionv[4];
  uint8_t optionc=0;
  if (freedom[0]) optionv[optionc++]=(struct fmn_raccoon_walk_option){freedom[0],FMN_DIR_W};
  if (freedom[1]) optionv[optionc++]=(struct fmn_raccoon_walk_option){freedom[1],FMN_DIR_E};
  if (freedom[2]) optionv[optionc++]=(struct fmn_raccoon_walk_option){freedom[2],FMN_DIR_N};
  if (freedom[3]) optionv[optionc++]=(struct fmn_raccoon_walk_option){freedom[3],FMN_DIR_S};
  
  // If there are no options, totally possible, then it doesn't matter which direction we pick.
  if (!optionc) {
    walkdir=FMN_DIR_S;
    return;
  }
  
  // Sort options long to short.
  #define COMPARE(a,b) if (b<optionc) { \
    if (optionv[a].freedom<optionv[b].freedom) { \
      struct fmn_raccoon_walk_option tmp=optionv[a]; \
      optionv[a]=optionv[b]; \
      optionv[b]=tmp; \
    } \
  }
  COMPARE(0,1)
  COMPARE(1,2)
  COMPARE(2,3)
  COMPARE(0,1)
  COMPARE(1,2)
  COMPARE(0,1)
  #undef COMPARE
  
  /* Make each option half as likely as the option before it.
   * We'll lean on the global frame count as a proxy for random numbers, because I am not allowing anything random.
   * The range of that "random" number then is (2**c-1), max 15.
   * Since it's one below a power of two, we can cheat and use BAN instead of MOD.
   */
  uint8_t range=(1<<optionc)-1;
  int8_t choice=fmn_play_frame_count&range;
  
  const struct fmn_raccoon_walk_option *p=optionv;
  uint8_t i=optionc;
  for (;i-->0;p++) {
    choice-=1<<i;
    if (choice<=0) {
      walkdir=p->dir;
      return;
    }
  }
  // This is impossible and I can prove it mathematically, buuuuut I'm not super good at math.
  walkdir=FMN_DIR_S;
}

/* Walking.
 */

static void fmn_raccoon_walk_end(struct fmn_sprite *sprite) {
  phase=FMN_RACCOON_PHASE_IDLE;
  phasetime=FMN_RACCOON_IDLE_TIME+(FMN_RACCOON_IDLE_TIME_EXTRA&(fmn_play_frame_count>>4));
}
 
static void fmn_raccoon_walk_animate(struct fmn_sprite *sprite) {
  animclock++;
  if (animclock>=6) {
    animclock=0;
    frame++;
    if (frame>=4) frame=0;
    switch (frame) {
      case 0: case 2: sprite->tileid=basetileid; break;
      case 1: sprite->tileid=basetileid+1; break;
      case 3: sprite->tileid=basetileid+2; break;
    }
  }
}

static void fmn_raccoon_walk_update(struct fmn_sprite *sprite) {

  int16_t dx=0,dy=0;
  switch (walkdir) {
    case FMN_DIR_N: dy=-FMN_RACCOON_WALK_SPEED; break;
    case FMN_DIR_S: dy=FMN_RACCOON_WALK_SPEED; break;
    case FMN_DIR_W: dx=-FMN_RACCOON_WALK_SPEED; break;
    case FMN_DIR_E: dx=FMN_RACCOON_WALK_SPEED; break;
  }
  int16_t x0=sprite->x,y0=sprite->y;
  sprite->x+=dx;
  sprite->y+=dy;
  
  // The screen box is a hard limit. Force it and end the walk phase if we strike.
  if (sprite->x<xlimit) {
    sprite->x=xlimit;
    fmn_raccoon_walk_end(sprite);
    return;
  }
  if (sprite->y<ylimit) {
    sprite->y=ylimit;
    fmn_raccoon_walk_end(sprite);
    return;
  }
  if (sprite->x>xlimit+FMN_SCREENW_MM-sprite->w) {
    sprite->x=xlimit+FMN_SCREENW_MM-sprite->w;
    fmn_raccoon_walk_end(sprite);
    return;
  }
  if (sprite->y>ylimit+FMN_SCREENH_MM-sprite->h) {
    sprite->y=ylimit+FMN_SCREENH_MM-sprite->h;
    fmn_raccoon_walk_end(sprite);
    return;
  }
  
  // Not on the screen edge, do a regular collision check and terminate phase if it resulted in zero motion.
  int16_t adjx,adjy;
  if (fmn_sprite_collide(&adjx,&adjy,sprite,FMN_TILE_SOLID|FMN_TILE_HOLE,FMN_SPRITE_FLAG_SOLID,1)) {
    if (!adjx&&!adjy) {
      sprite->x-=dx;
      sprite->y-=dy;
      fmn_raccoon_walk_end(sprite);
      return;
    }
    if ((sprite->x==x0)&&(sprite->y==y0)) {
      fmn_raccoon_walk_end(sprite);
      return;
    }
  }
}

/* Menace/throw.
 */
 
static void fmn_raccoon_menace_end(struct fmn_sprite *sprite) {
  phase=FMN_RACCOON_PHASE_IDLE;
  phasetime=FMN_RACCOON_IDLE_TIME+(FMN_RACCOON_IDLE_TIME_EXTRA&(fmn_play_frame_count>>4));
  if (missile) {
    // (sv[1],sv[2]) are (dx,dy) in fmn_sprtype_missile
    int16_t herox,heroy;
    fmn_hero_get_world_position_center(&herox,&heroy);
    float dx=herox-MISSILE->x;
    float dy=heroy-MISSILE->y;
    float distance=(int16_t)sqrtf(dx*dx+dy*dy);
    if (distance<1.0f) distance=1.0f;
    MISSILE->sv[1]=lround((dx*FMN_RACCOON_TOSS_SPEED)/distance);
    MISSILE->sv[2]=lround((dy*FMN_RACCOON_TOSS_SPEED)/distance);
  }
}

static uint8_t fmn_raccoon_can_throw(struct fmn_sprite *sprite) {
  if (charmed) return 0;
  // Generate missiles only when on screen. Which means (xlimit,ylimit) will match the global scroll position exactly.
  int16_t scrollx,scrolly;
  fmn_map_get_scroll_mm(&scrollx,&scrolly);
  if (scrollx!=xlimit) return 0;
  if (scrolly!=ylimit) return 0;
  return 1;
}

static void fmn_raccoon_generate_missile(struct fmn_sprite *sprite) {
  int16_t mx=sprite->x;
  int16_t my=sprite->y;
  if (sprite->xform&FMN_XFORM_XREV) mx+=FMN_MM_PER_TILE;
  if (!(missile=fmn_sprite_new(&fmn_sprtype_missile,0,mx,my,0,0,0))) return;
  MISSILE->image=sprite->image;
  MISSILE->tileid=(fmn_play_frame_count&16)?0x9e:0x9f;
  MISSILE->layer=sprite->layer+1;
}

/* Update.
 */

static void _fmn_raccoon_update(struct fmn_sprite *sprite) {

  if (phasetime) phasetime--;
  else switch (phase) {
    case FMN_RACCOON_PHASE_IDLE: fmn_raccoon_idle_end(sprite); break;
    case FMN_RACCOON_PHASE_WALK: fmn_raccoon_walk_end(sprite); break;
    case FMN_RACCOON_PHASE_MENACE: fmn_raccoon_menace_end(sprite); break;
  }

  switch (phase) {
    case FMN_RACCOON_PHASE_WALK: {
        fmn_raccoon_walk_animate(sprite);
        fmn_raccoon_walk_update(sprite);
      } break;
    case FMN_RACCOON_PHASE_MENACE: {
        animclock=0;
        frame=0;
        sprite->tileid=basetileid+3;
      } break;
    default: {
        animclock=0;
        frame=0;
        sprite->tileid=basetileid;
      }
  }
  
  // Always face the hero. Our natural image faces left.
  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  if (herox<sprite->x+(sprite->w>>1)) {
    sprite->xform=0;
  } else {
    sprite->xform=FMN_XFORM_XREV;
  }
}

/* Feather.
 */
 
static uint8_t _fmn_raccoon_featherspell(struct fmn_sprite *sprite,const uint8_t *v,uint8_t c) {
  charmed=1;
  return 1;
}

/* Type definition.
 */

const struct fmn_sprtype fmn_sprtype_raccoon={
  .name="raccoon",
  .init=_fmn_raccoon_init,
  .update=_fmn_raccoon_update,
  .render=fmn_sprite_render_default,
  .featherspell=_fmn_raccoon_featherspell,
};
