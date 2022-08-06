#include "game/fullmoon.h"
#include "game/model/fmn_map.h"
#include "game/sprite/fmn_sprite.h"
#include "game/sprite/hero/fmn_hero.h"

#define movec sprite->bv[0]
#define movex sprite->sv[0]
#define movey sprite->sv[1]

#define STATUE_FLOAT_SPEED 1

// Must be a factor of FMN_MM_PER_TILE.
#define SPEED 2

static int8_t _statue_init(struct fmn_sprite *sprite,const struct fmn_sprdef *sprdef) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  return 0;
}

static void fmn_statue_float(struct fmn_sprite *sprite,int8_t dx,int8_t dy) {
  sprite->x+=dx*STATUE_FLOAT_SPEED;
  sprite->y+=dy*STATUE_FLOAT_SPEED;
  fmn_sprite_collide(0,0,sprite,FMN_TILE_SOLID,FMN_SPRITE_FLAG_SOLID,1);
}

static void fmn_statue_tsu_update(struct fmn_sprite *sprite) {
  uint8_t herodir=fmn_hero_get_feather_dir();
  if (!herodir) return;
  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  switch (herodir) {
    case FMN_DIR_E: {
        if ((heroy<sprite->y)||(heroy>=sprite->y+sprite->h)) return;
        if (herox>sprite->x) return;
        fmn_statue_float(sprite,-1,0);
      } break;
    case FMN_DIR_W: {
        if ((heroy<sprite->y)||(heroy>=sprite->y+sprite->h)) return;
        if (herox<sprite->x) return;
        fmn_statue_float(sprite,1,0);
      } break;
    case FMN_DIR_S: {
        if ((herox<sprite->x)||(herox>=sprite->x+sprite->w)) return;
        if (heroy>sprite->y) return;
        fmn_statue_float(sprite,0,-1);
      } break;
    case FMN_DIR_N: {
        if ((herox<sprite->x)||(herox>=sprite->x+sprite->w)) return;
        if (heroy<sprite->y) return;
        fmn_statue_float(sprite,0,1);
      } break;
  }
}

static void _statue_update(struct fmn_sprite *sprite) {
  if (sprite->tileid==0xb3) {
    fmn_statue_tsu_update(sprite);
  } else if (movec&&(movex||movey)) {
    movec--;
    sprite->x+=movex;
    sprite->y+=movey;
    int16_t adjx,adjy;
    if (fmn_sprite_collide(
      &adjx,&adjy,sprite,
      (FMN_TILE_SOLID|FMN_TILE_HOLE),
      FMN_SPRITE_FLAG_SOLID,
      1
    )) {
      if (!adjx&&!adjy) {
        sprite->x-=movex;
        sprite->y-=movey;
      }
      movex=0;
      movey=0;
    }
  }
}

static uint8_t _statue_featherspell(struct fmn_sprite *sprite,const uint8_t *v,uint8_t c) {
  switch (sprite->tileid) {
  
    case 0xb0: { // gamma aka octopus
        if ((c>=3)&&(v[c-3]==FMN_DIR_W)&&(v[c-2]==FMN_DIR_E)&&(v[c-1]==FMN_DIR_W)) {
          //TODO some kind of "poof!"
          fmn_sprite_del_later(sprite);
          return 1;
        }
      } return 0;
      
    case 0xb1: { // to aka mermaid
        if (c>=4) {
          uint8_t move=0;
          switch (v[c-4]) {
            case FMN_DIR_W: if ((v[c-3]==FMN_DIR_N)&&(v[c-2]==FMN_DIR_E)&&(v[c-1]==FMN_DIR_S)) move=FMN_DIR_N; break;
            case FMN_DIR_N: if ((v[c-3]==FMN_DIR_E)&&(v[c-2]==FMN_DIR_S)&&(v[c-1]==FMN_DIR_W)) move=FMN_DIR_E; break;
            case FMN_DIR_E: if ((v[c-3]==FMN_DIR_S)&&(v[c-2]==FMN_DIR_W)&&(v[c-1]==FMN_DIR_N)) move=FMN_DIR_S; break;
            case FMN_DIR_S: if ((v[c-3]==FMN_DIR_W)&&(v[c-2]==FMN_DIR_N)&&(v[c-1]==FMN_DIR_E)) move=FMN_DIR_W; break;
          }
          switch (move) {
            case FMN_DIR_W: movex=-SPEED; movey=     0; movec=FMN_MM_PER_TILE/SPEED; return 1;
            case FMN_DIR_N: movex=     0; movey=-SPEED; movec=FMN_MM_PER_TILE/SPEED; return 1;
            case FMN_DIR_E: movex= SPEED; movey=     0; movec=FMN_MM_PER_TILE/SPEED; return 1;
            case FMN_DIR_S: movex=     0; movey= SPEED; movec=FMN_MM_PER_TILE/SPEED; return 1;
          }
        }
      } return 0;
      
    case 0xb2: { // aa aka lobster
        if (c>=5) {
          if ((v[c-5]==v[c-4])&&(v[c-4]==v[c-3])&&(v[c-3]==v[c-1])&&(v[c-2]!=v[c-1])) switch (v[c-2]) {
            case FMN_DIR_W: movex=-SPEED; movey=     0; movec=FMN_MM_PER_TILE/SPEED; return 1;
            case FMN_DIR_N: movex=     0; movey=-SPEED; movec=FMN_MM_PER_TILE/SPEED; return 1;
            case FMN_DIR_E: movex= SPEED; movey=     0; movec=FMN_MM_PER_TILE/SPEED; return 1;
            case FMN_DIR_S: movex=     0; movey= SPEED; movec=FMN_MM_PER_TILE/SPEED; return 1;
          }
        }
      } return 0;
      
    case 0xb3: { // tsu aka fish
        // not a feather spell; summon the statue from a distance
      } return 0;
      
  }
  return 0;
}

const struct fmn_sprtype fmn_sprtype_statue={
  .name="statue",
  .init=_statue_init,
  .update=_statue_update,
  .render=fmn_sprite_render_default,
  .featherspell=_statue_featherspell,
};
