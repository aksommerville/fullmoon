#include "game/fullmoon.h"
#include "game/fmn_data.h"
#include "game/ui/fmn_play.h"
#include "game/model/fmn_map.h"
#include "game/sprite/hero/fmn_hero.h"
#include "game/sprite/fmn_sprite.h"
#include <math.h>

#define stage sprite->bv[3]
#define animclock sprite->bv[4]
#define animframe sprite->bv[5]
#define hazard_blackout sprite->bv[6]
#define chargec sprite->bv[7]
#define dy sprite->sv[0]
#define stagetime sprite->sv[1] /* reset at stage changes, counts up */

#define FMN_WEREWOLF_STAGE_WALK 0 /* walk upright, up and down */
#define FMN_WEREWOLF_STAGE_SUMMON 1 /* generating a fireball */
#define FMN_WEREWOLF_STAGE_SHOOT 2 /* briefly, dispatching the fireball */
#define FMN_WEREWOLF_STAGE_WHINE 3 /* hit by reflected wolfball, reacting before transition to all fours */
#define FMN_WEREWOLF_STAGE_GROWL 4 /* on all fours, warning of an imminent charge */
#define FMN_WEREWOLF_STAGE_CHARGE 5 /* running horizontally */
#define FMN_WEREWOLF_STAGE_DOGWALK 6 /* like WALK but we're on all fours */
#define FMN_WEREWOLF_STAGE_DEAD 7

#define FMN_WEREWOLF_WALK_SPEED 2
#define FMN_WEREWOLF_FIREBALL_SPEED 8
#define FMN_WEREWOLF_CHARGE_SPEED 6
#define FMN_WEREWOLF_BUTT_MARGIN (FMN_MM_PER_TILE)

static void fmn_werewolf_begin_WALK(struct fmn_sprite *sprite);
static void fmn_werewolf_begin_SUMMON(struct fmn_sprite *sprite);
static void fmn_werewolf_begin_SHOOT(struct fmn_sprite *sprite);
static void fmn_werewolf_begin_WHINE(struct fmn_sprite *sprite);
static void fmn_werewolf_begin_GROWL(struct fmn_sprite *sprite);
static void fmn_werewolf_begin_CHARGE(struct fmn_sprite *sprite);
static void fmn_werewolf_begin_DOGWALK(struct fmn_sprite *sprite);
static void fmn_werewolf_begin_DEAD(struct fmn_sprite *sprite);

/* Setup.
 */
 
static int8_t _werewolf_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {

  // We initially occupy the requested cell, and its neighbors east and south.
  sprite->w=FMN_MM_PER_TILE*2;
  sprite->h=FMN_MM_PER_TILE*2;
  sprite->x-=FMN_MM_PER_TILE>>1;
  sprite->y-=FMN_MM_PER_TILE>>1;
  
  stage=FMN_WEREWOLF_STAGE_WALK;
  dy=1;
  
  if (fmn_game_get_state()&FMN_STATE_WOLF_DEAD) {
    // Don't use fmn_werewolf_begin_DEAD(); that triggers all the fireworks.
    stage=FMN_WEREWOLF_STAGE_DEAD;
    sprite->w=FMN_MM_PER_TILE*3;
    sprite->flags=0;
  }
  
  return 0;
}

/* Check hero collisions.
 */
 
static void fmn_werewolf_check_hero(struct fmn_sprite *sprite) {
  if (stage==FMN_WEREWOLF_STAGE_DEAD) return;
  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  if (herox<sprite->x) return;
  if (heroy<sprite->y) return;
  if (herox>=sprite->x+sprite->w) return;
  if (heroy>=sprite->y+sprite->h) return;
  fmn_hero_injure(sprite);
}

/* Check hazards. Only relevant in stages WALK, SUMMON, SHOOT.
 */
 
static void fmn_werewolf_check_hazards(struct fmn_sprite *sprite) {
  if (hazard_blackout) return;
  struct fmn_sprite **p=fmn_spritev;
  uint16_t i=fmn_spritec;
  for (;i-->0;p++) {
    struct fmn_sprite *q=*p;
    if (q->type!=&fmn_sprtype_missile) continue;
    int16_t qx=q->x+(q->w>>1);
    int16_t qy=q->y+(q->h>>1);
    if (qx<sprite->x) continue;
    if (qy<sprite->y) continue;
    if (qx>=sprite->x+sprite->w) continue;
    if (qy>=sprite->y+sprite->h) continue;
    fmn_werewolf_begin_WHINE(sprite);
    return;
  }
}

/* SHOOT stage.
 */
 
static void fmn_werewolf_begin_SHOOT(struct fmn_sprite *sprite) {
  sprite->w=FMN_MM_PER_TILE*2;
  stage=FMN_WEREWOLF_STAGE_SHOOT;
  stagetime=0;
  hazard_blackout=30; // give enough time for the wolfball to escape its initial collision with us
  
  int16_t x=sprite->x;
  if (sprite->xform&FMN_XFORM_XREV) x+=sprite->w;
  int16_t y=sprite->y+(sprite->h>>1);
  struct fmn_sprite *wolfball=fmn_sprite_new(&fmn_sprtype_missile,0,x,y,0,0,0);
  if (wolfball) {
    // (sv[1],sv[2]) are (dx,dy); bv[0] is animclock, which we must set to enable
    wolfball->image=sprite->image;
    wolfball->tileid=0x0d;
    wolfball->bv[0]=1;
    int16_t herox,heroy;
    fmn_hero_get_world_position_center(&herox,&heroy);
    float distance=sqrtf((herox-x)*(herox-x)+(heroy-y)*(heroy-y));
    wolfball->sv[1]=((herox-x)*FMN_WEREWOLF_FIREBALL_SPEED)/distance;
    wolfball->sv[2]=((heroy-y)*FMN_WEREWOLF_FIREBALL_SPEED)/distance;
    // Sign of (dx) must agree with the direction the wolf is facing.
    if (sprite->xform&FMN_XFORM_XREV) {
      if (wolfball->sv[1]<0) wolfball->sv[1]=-wolfball->sv[1];
    } else {
      if (wolfball->sv[1]>0) wolfball->sv[1]=-wolfball->sv[1];
    }
  }
}

static void fmn_werewolf_update_SHOOT(struct fmn_sprite *sprite) {
  if (stagetime>=60) {
    fmn_werewolf_begin_WALK(sprite);
  }
  fmn_werewolf_check_hazards(sprite);
}

/* SUMMON stage.
 */
 
static void fmn_werewolf_begin_SUMMON(struct fmn_sprite *sprite) {
  sprite->w=FMN_MM_PER_TILE*2;
  stage=FMN_WEREWOLF_STAGE_SUMMON;
  stagetime=0;
  animclock=0;
  animframe=0;
}

static void fmn_werewolf_update_SUMMON(struct fmn_sprite *sprite) {
  if (animclock) animclock--;
  else {
    animclock=5;
    animframe++;
    if (animframe>=4) animframe=0;
  }
  if (stagetime>=90) {
    fmn_werewolf_begin_SHOOT(sprite);
  }
  fmn_werewolf_check_hazards(sprite);
}

/* WALK stage.
 */

static void fmn_werewolf_begin_WALK(struct fmn_sprite *sprite) {
  sprite->w=FMN_MM_PER_TILE*2;
  stage=FMN_WEREWOLF_STAGE_WALK;
  stagetime=0;
}
 
static void fmn_werewolf_update_WALK(struct fmn_sprite *sprite) {

  if (animclock) animclock--;
  else {
    animclock=7;
    animframe++;
    if (animframe>=4) animframe=0;
  }
  
  sprite->y+=dy*FMN_WEREWOLF_WALK_SPEED;
  if (fmn_sprite_collide(0,0,sprite,FMN_TILE_SOLID|FMN_TILE_HOLE,0,1)) {
    dy=-dy;
  }
  
  // Generate a fireball after a specific interval.
  if (stagetime>=300) {
    fmn_werewolf_begin_SUMMON(sprite);
  }
  
  fmn_werewolf_check_hazards(sprite);
}

/* WHINE and GROWL stages -- decorative.
 */
 
static void fmn_werewolf_begin_WHINE(struct fmn_sprite *sprite) {
  sprite->w=FMN_MM_PER_TILE*2;
  stage=FMN_WEREWOLF_STAGE_WHINE;
  stagetime=0;
  animclock=0;
  animframe=0;
  chargec=0; // reset this counter, which will eventually cause us to rise to hind legs again
}

static void fmn_werewolf_update_WHINE(struct fmn_sprite *sprite) {
  if (stagetime>=30) {
    fmn_werewolf_begin_GROWL(sprite);
  }
}
 
static void fmn_werewolf_begin_GROWL(struct fmn_sprite *sprite) {
  sprite->w=FMN_MM_PER_TILE*3;
  stage=FMN_WEREWOLF_STAGE_GROWL;
  stagetime=0;
  animclock=0;
  animframe=0;
}

static void fmn_werewolf_update_GROWL(struct fmn_sprite *sprite) {
  if (stagetime>=60) {
    fmn_werewolf_begin_CHARGE(sprite);
  }
}

/* CHARGE stage.
 */
 
static void fmn_werewolf_begin_CHARGE(struct fmn_sprite *sprite) {
  sprite->w=FMN_MM_PER_TILE*3;
  stage=FMN_WEREWOLF_STAGE_CHARGE;
  stagetime=0;
  animclock=0;
  animframe=0;
  chargec++;
}

static void fmn_werewolf_update_CHARGE(struct fmn_sprite *sprite) {
  
  if (animclock) animclock--;
  else {
    animclock=6;
    animframe++;
    if (animframe>=2) animframe=0;
  }
  
  if (sprite->xform&FMN_XFORM_XREV) sprite->x+=FMN_WEREWOLF_CHARGE_SPEED;
  else sprite->x-=FMN_WEREWOLF_CHARGE_SPEED;
  if (fmn_sprite_collide(0,0,sprite,FMN_TILE_SOLID|FMN_TILE_HOLE,0,1)) {
    sprite->xform^=FMN_XFORM_XREV;
    fmn_werewolf_begin_DOGWALK(sprite);
  }
}

/* DOGWALK stage.
 */
 
static void fmn_werewolf_begin_DOGWALK(struct fmn_sprite *sprite) {
  sprite->w=FMN_MM_PER_TILE*3;
  stage=FMN_WEREWOLF_STAGE_DOGWALK;
  stagetime=0;
  animclock=0;
  animframe=0;
}

static void fmn_werewolf_update_DOGWALK(struct fmn_sprite *sprite) {

  if (animclock) animclock--;
  else {
    animclock=7;
    animframe++;
    if (animframe>=4) animframe=0;
  }
  
  // Back off from the edge -- we must be a good sport and leave enough room for the witch back there.
  int16_t scrollx,scrolly;
  fmn_map_get_scroll_mm(&scrollx,&scrolly);
  if (sprite->x<scrollx+FMN_WEREWOLF_BUTT_MARGIN) {
    sprite->x+=1;
  } else if (sprite->x+sprite->w>scrollx+FMN_SCREENW_MM-FMN_WEREWOLF_BUTT_MARGIN) {
    sprite->x-=1;
  }
  
  sprite->y+=dy*FMN_WEREWOLF_WALK_SPEED;
  if (fmn_sprite_collide(0,0,sprite,FMN_TILE_SOLID|FMN_TILE_HOLE,0,1)) {
    dy=-dy;
  }
  
  // Begin charge after some interval.
  if (stagetime>=250) {
    if (chargec>=3) { // stand back up. she had her chance and blew it...
      fmn_werewolf_begin_SUMMON(sprite);
    } else {
      fmn_werewolf_begin_GROWL(sprite);
    }
  }
}

/* DEAD stage.
 */
 
static void fmn_werewolf_begin_DEAD(struct fmn_sprite *sprite) {
  sprite->w=FMN_MM_PER_TILE*3;
  stage=FMN_WEREWOLF_STAGE_DEAD;
  stagetime=0;
  sprite->flags=0; // no more HAZARD
  
  fmn_game_set_state(FMN_STATE_WOLF_DEAD,FMN_STATE_WOLF_DEAD);
  fmn_game_win();
  
  // The seven circles of a werewolf's soul.
  int16_t x=sprite->x+(sprite->w>>1);
  int16_t y=sprite->y+(sprite->h>>1);
  uint8_t i=7; while (i-->0) {
    struct fmn_sprite *soulball=fmn_sprite_new(&fmn_sprtype_soulball,0,x,y,i,7,0);
  }
}

/* Update.
 */
 
static void _werewolf_update(struct fmn_sprite *sprite) {

  // Do nothing if we're offscreen.
  int16_t scrollx,scrolly;
  fmn_map_get_scroll_mm(&scrollx,&scrolly);
  if (sprite->x+sprite->w<scrollx) return;
  if (sprite->y+sprite->h<scrolly) return;
  if (sprite->x>=scrollx+FMN_SCREENW_MM) return;
  if (sprite->y>=scrolly+FMN_SCREENH_MM) return;
  
  stagetime++;
  if (hazard_blackout) hazard_blackout--;
  fmn_werewolf_check_hero(sprite);

  switch (stage) {
    case FMN_WEREWOLF_STAGE_WALK: fmn_werewolf_update_WALK(sprite); break;
    case FMN_WEREWOLF_STAGE_SUMMON: fmn_werewolf_update_SUMMON(sprite); break;
    case FMN_WEREWOLF_STAGE_SHOOT: fmn_werewolf_update_SHOOT(sprite); break;
    case FMN_WEREWOLF_STAGE_WHINE: fmn_werewolf_update_WHINE(sprite); break;
    case FMN_WEREWOLF_STAGE_GROWL: fmn_werewolf_update_GROWL(sprite); break;
    case FMN_WEREWOLF_STAGE_CHARGE: fmn_werewolf_update_CHARGE(sprite); break;
    case FMN_WEREWOLF_STAGE_DOGWALK: fmn_werewolf_update_DOGWALK(sprite); break;
  }
}

/* Render, WALK stage.
 */
 
static void fmn_werewolf_render_WALK(
  struct fmn_image *dst,
  int16_t dstx,int16_t dsty,
  struct fmn_sprite *sprite
) {
  int16_t srcx=0;
  int16_t srcy=0;
  switch (animframe) {
    case 0: case 2: srcx=0; break;
    case 1: srcx=FMN_TILESIZE*2; break;
    case 3: srcx=FMN_TILESIZE*4; break;
  }
  fmn_blit(dst,dstx,dsty,sprite->image,srcx,srcy,FMN_TILESIZE*2,FMN_TILESIZE*2,sprite->xform);
}

/* Render, SUMMON stage.
 */
 
static void fmn_werewolf_render_SUMMON(
  struct fmn_image *dst,
  int16_t dstx,int16_t dsty,
  struct fmn_sprite *sprite
) {
  fmn_blit(dst,dstx,dsty,sprite->image,FMN_TILESIZE*6,0,FMN_TILESIZE*2,FMN_TILESIZE*2,sprite->xform);
  int16_t ballx=dstx;
  int16_t bally=dsty-2*FMN_GFXSCALE;
  if (sprite->xform&FMN_XFORM_XREV) {
    ballx-=2*FMN_GFXSCALE;
  } else {
    ballx+=FMN_TILESIZE+2*FMN_GFXSCALE;
  }
  uint8_t tileid=0x0d;
  switch (animframe) {
    case 1: case 3: tileid+=1; break;
    case 2: tileid+=2; break;
  }
  fmn_blit_tile(dst,ballx,bally,sprite->image,tileid,0);
}

/* Render, trivial stages.
 */
 
static void fmn_werewolf_render_SHOOT(
  struct fmn_image *dst,
  int16_t dstx,int16_t dsty,
  struct fmn_sprite *sprite
) {
  fmn_blit(dst,dstx,dsty,sprite->image,FMN_TILESIZE*8,0,FMN_TILESIZE*2,FMN_TILESIZE*2,sprite->xform);
}
 
static void fmn_werewolf_render_WHINE(
  struct fmn_image *dst,
  int16_t dstx,int16_t dsty,
  struct fmn_sprite *sprite
) {
  fmn_blit(dst,dstx,dsty,sprite->image,0,FMN_TILESIZE*4,FMN_TILESIZE*2,FMN_TILESIZE*2,sprite->xform);
}
 
static void fmn_werewolf_render_GROWL(
  struct fmn_image *dst,
  int16_t dstx,int16_t dsty,
  struct fmn_sprite *sprite
) {
  fmn_blit(dst,dstx,dsty,sprite->image,FMN_TILESIZE*3,FMN_TILESIZE*2,FMN_TILESIZE*3,FMN_TILESIZE*2,sprite->xform);
}
 
static void fmn_werewolf_render_DEAD(
  struct fmn_image *dst,
  int16_t dstx,int16_t dsty,
  struct fmn_sprite *sprite
) {
  fmn_blit(dst,dstx,dsty,sprite->image,FMN_TILESIZE*10,0,FMN_TILESIZE*3,FMN_TILESIZE*2,sprite->xform);
}

/* Render, CHARGE stage.
 */
 
static void fmn_werewolf_render_CHARGE(
  struct fmn_image *dst,
  int16_t dstx,int16_t dsty,
  struct fmn_sprite *sprite
) {
  int16_t srcx=0;
  int16_t srcy=FMN_TILESIZE*2;
  switch (animframe) {
    case 0: srcx=0; break;
    case 1: srcx=FMN_TILESIZE*6; break;
  }
  fmn_blit(dst,dstx,dsty,sprite->image,srcx,srcy,FMN_TILESIZE*3,FMN_TILESIZE*2,sprite->xform);
}

/* Render, DOGWALK stage.
 */
 
static void fmn_werewolf_render_DOGWALK(
  struct fmn_image *dst,
  int16_t dstx,int16_t dsty,
  struct fmn_sprite *sprite
) {
  int16_t srcx=0;
  int16_t srcy=FMN_TILESIZE*2;
  switch (animframe) {
    case 0: case 2: srcx=0; break;
    case 1: srcx=FMN_TILESIZE*9; break;
    case 3: srcx=FMN_TILESIZE*12; break;
  }
  fmn_blit(dst,dstx,dsty,sprite->image,srcx,srcy,FMN_TILESIZE*3,FMN_TILESIZE*2,sprite->xform);
}

/* Render.
 */
 
static void _werewolf_render(
  struct fmn_image *dst,
  int16_t scrollx,int16_t scrolly,
  struct fmn_sprite *sprite
) {
  int16_t dstx=((sprite->x-scrollx)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  int16_t dsty=((sprite->y-scrolly)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  switch (stage) {
    case FMN_WEREWOLF_STAGE_WALK: fmn_werewolf_render_WALK(dst,dstx,dsty,sprite); break;
    case FMN_WEREWOLF_STAGE_SUMMON: fmn_werewolf_render_SUMMON(dst,dstx,dsty,sprite); break;
    case FMN_WEREWOLF_STAGE_SHOOT: fmn_werewolf_render_SHOOT(dst,dstx,dsty,sprite); break;
    case FMN_WEREWOLF_STAGE_WHINE: fmn_werewolf_render_WHINE(dst,dstx,dsty,sprite); break;
    case FMN_WEREWOLF_STAGE_GROWL: fmn_werewolf_render_GROWL(dst,dstx,dsty,sprite); break;
    case FMN_WEREWOLF_STAGE_CHARGE: fmn_werewolf_render_CHARGE(dst,dstx,dsty,sprite); break;
    case FMN_WEREWOLF_STAGE_DOGWALK: fmn_werewolf_render_DOGWALK(dst,dstx,dsty,sprite); break;
    case FMN_WEREWOLF_STAGE_DEAD: fmn_werewolf_render_DEAD(dst,dstx,dsty,sprite); break;
  }
}

/* Feather.
 */
 
static uint8_t _werewolf_featherspell(struct fmn_sprite *sprite,const uint8_t *v,uint8_t c) {
  if (c<1) return 0;
  // Only relevant when we're on all fours.
  switch (stage) {
    case FMN_WEREWOLF_STAGE_GROWL:
    case FMN_WEREWOLF_STAGE_CHARGE:
    case FMN_WEREWOLF_STAGE_DOGWALK:
      break;
    default: return 0;
  }
  // Must tickle butt.
  if (sprite->xform&FMN_XFORM_XREV) {
    if (v[c-1]!=FMN_DIR_W) return 0;
  } else {
    if (v[c-1]!=FMN_DIR_E) return 0;
  }
  fmn_werewolf_begin_DEAD(sprite);
  return 1;
}

/* Type definition.
 */
 
const struct fmn_sprtype fmn_sprtype_werewolf={
  .name="werewolf",
  .init=_werewolf_init,
  .update=_werewolf_update,
  .render=_werewolf_render,
  .featherspell=_werewolf_featherspell,
};
