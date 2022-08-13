#include "game/fullmoon.h"
#include "game/sprite/hero/fmn_hero.h"
#include "game/sprite/fmn_sprite.h"
#include <math.h>

#define awake sprite->bv[3]
#define charmed sprite->bv[4]
#define breathing sprite->bv[5] /* counts down after shooting a fireball */
#define framec sprite->sv[0] /* for animation, shared across all faces */
#define elevation sprite->sv[1] /* mm, how far off the ground, ie how much foot exposed */
#define firetime sprite->sv[2] /* counts down during walking, to next fireball */
#define leftlimit sprite->sv[3]
#define rightlimit sprite->sv[4]
#define dx sprite->sv[5]

#define FMN_ROCKGUARD_AWAKE_ELEVATION ((3*FMN_MM_PER_TILE)/FMN_TILESIZE)
#define FMN_ROCKGUARD_WAKE_SPEED 1
#define FMN_ROCKGUARD_CHARMED_WALK_SPEED 1
#define FMN_ROCKGUARD_LEG_PERIOD 60
#define FMN_ROCKGUARD_BREATHING_TIME 90
#define FMN_ROCKGUARD_WALKING_TIME 180
#define FMN_ROCKGUARD_WALK_SPEED 1
#define FMN_ROCKGUARD_FIREBALL_SPEED 6

/* Setup.
 */
 
static int8_t _rockguard_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  sprite->x-=FMN_MM_PER_TILE>>1;
  sprite->y-=FMN_MM_PER_TILE>>1;
  sprite->w=FMN_MM_PER_TILE*2;
  sprite->h=FMN_MM_PER_TILE*3;
  
  leftlimit=sprite->x;
  rightlimit=sprite->x+FMN_MM_PER_TILE*3;
  dx=1;
  
  return 0;
}

/* If the hero is butted against my edge, wake up.
 */
 
static void fmn_rockguard_check_wakeup(struct fmn_sprite *sprite) {
  const struct fmn_sprite *hero=fmn_hero_get_sprite();
  if (!hero) return;
  // Beware, it's "<" ">" and not "<=" ">=", and it matters.
  if (hero->x+hero->w<sprite->x) return;
  if (hero->y+hero->h<sprite->y) return;
  if (hero->x>sprite->x+sprite->w) return;
  if (hero->y>sprite->y+sprite->h) return;
  awake=1;
}

/* Generate a fireball.
 */
 
static void fmn_rockguard_generate_fireball(struct fmn_sprite *sprite) {
  int16_t x=sprite->x+FMN_MM_PER_TILE+(FMN_MM_PER_TILE>>1);
  int16_t y=sprite->y+FMN_MM_PER_TILE*2+(FMN_MM_PER_TILE>>1);
  struct fmn_sprite *fireball=fmn_sprite_new(&fmn_sprtype_missile,0,x,y,0,0,0);
  if (fireball) {
    // (sv[1],sv[2]) are (dx,dy); bv[0] is animclock, which we must set to enable
    fireball->image=sprite->image;
    fireball->tileid=0x0d;
    fireball->bv[0]=1;
    int16_t herox,heroy;
    fmn_hero_get_world_position_center(&herox,&heroy);
    float distance=sqrtf((herox-x)*(herox-x)+(heroy-y)*(heroy-y));
    fireball->sv[1]=((herox-x)*FMN_ROCKGUARD_FIREBALL_SPEED)/distance;
    fireball->sv[2]=((heroy-y)*FMN_ROCKGUARD_FIREBALL_SPEED)/distance;
    if (fireball->sv[1]<0) fireball->sv[1]=-fireball->sv[1];
  }
}

/* Update.
 */
 
static void _rockguard_update(struct fmn_sprite *sprite) {
  framec++;

  // Sleeping.
  if (!awake) {
    elevation=0;
    fmn_rockguard_check_wakeup(sprite);
    return;
  }
  
  // Rising.
  if (elevation<FMN_ROCKGUARD_AWAKE_ELEVATION) {
    elevation+=FMN_ROCKGUARD_WAKE_SPEED;
    if (elevation>=FMN_ROCKGUARD_AWAKE_ELEVATION) {
      elevation=FMN_ROCKGUARD_AWAKE_ELEVATION;
      firetime=FMN_ROCKGUARD_WALKING_TIME;
    }
    return;
  }
  
  // Breathing fire.
  if (breathing) {
    breathing--;
    if (!breathing) {
      firetime=FMN_ROCKGUARD_WALKING_TIME;
    }
    return;
  }
  
  // Charmed.
  if (charmed) {
    sprite->x+=FMN_ROCKGUARD_CHARMED_WALK_SPEED;
    fmn_sprite_collide(0,0,sprite,FMN_TILE_SOLID|FMN_TILE_HOLE,FMN_SPRITE_FLAG_SOLID,1);
    return;
  }
  
  // Shoot a fireball?
  if (firetime) firetime--;
  else {
    breathing=FMN_ROCKGUARD_BREATHING_TIME;
    fmn_rockguard_generate_fireball(sprite);
    return;
  }
  
  // Walk back and forth.
  sprite->x+=dx*FMN_ROCKGUARD_WALK_SPEED;
  if ((sprite->x<leftlimit)||(sprite->x>rightlimit)) {
    sprite->x-=dx*FMN_ROCKGUARD_WALK_SPEED;
    dx=-dx;
  } else if (fmn_sprite_collide(0,0,sprite,FMN_TILE_SOLID|FMN_TILE_HOLE,FMN_SPRITE_FLAG_SOLID,1)) {
    dx=-dx;
  }
}

/* Spells.
 */
 
static uint8_t _rockguard_featherspell(struct fmn_sprite *sprite,const uint8_t *v,uint8_t c) {
  if (!awake) return 0;
  if (c<1) return 0;
  if (v[c-1]!=FMN_DIR_E) return 0;
  charmed=1;
  breathing=0;
  return 1;
}

static void _rockguard_animate(struct fmn_sprite *sprite) {
  // "Coming to life" and "waking up" are basically the same thing, why not.
  awake=1;
}

/* Render.
 */
 
static void _rockguard_render(
  struct fmn_image *dst,
  int16_t scrollx,int16_t scrolly,
  struct fmn_sprite *sprite
) {
  int16_t dstx=((sprite->x-scrollx)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  int16_t dsty=((sprite->y-scrolly-elevation)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  int16_t bottom=((sprite->y+sprite->h-scrolly)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  int16_t srcx=(sprite->tileid&15)*FMN_TILESIZE;
  int16_t srcy=(sprite->tileid>>4)*FMN_TILESIZE;
  
  // The default 6 tiles are the complete sleeping pose. Draw it in one shot and we're done.
  if (!awake) {
    fmn_blit(dst,dstx,dsty,sprite->image,srcx,srcy,FMN_TILESIZE*2,FMN_TILESIZE*3,0);
    return;
  }
  
  // Feet if elevated at all.
  if (elevation) {
    int16_t footy=bottom-FMN_TILESIZE;
    uint8_t tileid=sprite->tileid+0x14; // flat-footed, normal.
    int16_t llift=0,rlift=0;
    if (elevation<FMN_ROCKGUARD_AWAKE_ELEVATION) tileid-=0x10; // tiptoes, rising.
    else if (!breathing) { // Walking, animate up and down
      int16_t phase=framec%FMN_ROCKGUARD_LEG_PERIOD;
      int16_t range=FMN_ROCKGUARD_AWAKE_ELEVATION+10; // fudge it out a little so it stays at the top more than one frame
      llift=(phase*range*2)/FMN_ROCKGUARD_LEG_PERIOD;
      if (llift>=range) {
        llift=range*2-llift;
      }
      rlift=range-llift;
      llift=(llift*FMN_TILESIZE)/FMN_MM_PER_TILE;
      rlift=(rlift*FMN_TILESIZE)/FMN_MM_PER_TILE;
    }
    fmn_blit_tile(dst,dstx,footy-llift,sprite->image,tileid,0);
    fmn_blit_tile(dst,dstx+FMN_TILESIZE,footy-rlift,sprite->image,tileid,0);
  }
  
  // Four tiles in a gamma shape, which do not change.
  fmn_blit_tile(dst,dstx,dsty,sprite->image,sprite->tileid,0);
  fmn_blit_tile(dst,dstx+FMN_TILESIZE,dsty,sprite->image,sprite->tileid+0x01,0);
  fmn_blit_tile(dst,dstx,dsty+FMN_TILESIZE*1,sprite->image,sprite->tileid+0x10,0);
  fmn_blit_tile(dst,dstx,dsty+FMN_TILESIZE*2,sprite->image,sprite->tileid+0x20,0);
  
  // Eye. +0x11=sleep, +0x02=forward, +0x03=charmed, +0x12=surprised, +0x13=strain
  uint8_t tileid=sprite->tileid;
  if (elevation<FMN_ROCKGUARD_AWAKE_ELEVATION) tileid+=0x13;
  else if (charmed) tileid+=0x03;
  else if (breathing) tileid+=0x12;
  else tileid+=0x02;
  fmn_blit_tile(dst,dstx+FMN_TILESIZE,dsty+FMN_TILESIZE,sprite->image,tileid,0);
  
  // Mouth. +0x21=neutral, +0x22=strain, +0x23=charmed, +0x24=fireball
  if (elevation<FMN_ROCKGUARD_AWAKE_ELEVATION) tileid=sprite->tileid+0x22;
  else if (charmed) tileid=sprite->tileid+0x23;
  else if (breathing) tileid=sprite->tileid+0x24;
  else tileid=sprite->tileid+0x21;
  fmn_blit_tile(dst,dstx+FMN_TILESIZE,dsty+FMN_TILESIZE*2,sprite->image,tileid,0);
}

/* Type definition.
 */
 
const struct fmn_sprtype fmn_sprtype_rockguard={
  .name="rockguard",
  .init=_rockguard_init,
  .update=_rockguard_update,
  .featherspell=_rockguard_featherspell,
  .render=_rockguard_render,
  .spell_animate=_rockguard_animate,
};
