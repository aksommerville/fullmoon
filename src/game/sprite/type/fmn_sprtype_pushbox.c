#include "game/fullmoon.h"
#include "game/model/fmn_map.h"
#include "game/sprite/fmn_sprite.h"
#include "game/sprite/hero/fmn_hero.h"
#include <stdlib.h>
#include <stdio.h>

#define movec sprite->bv[0]
#define charmdir sprite->bv[1]
#define movex sprite->sv[0]
#define movey sprite->sv[1]

#define PUSHBOX_CHARM_SPEED 2

// Must be a factor of FMN_MM_PER_TILE.
#define SPEED 2

static int8_t _pushbox_init(struct fmn_sprite *sprite,const struct fmn_sprdef *sprdef) {
  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  return 0;
}

// We are doing charmed motion and a collision occurred.
// Step at minimum speed toward the nearest cell boundary on the cross axis.
// So the player doesn't need to get perfect alignment for 2D navigation, just within half a cell.
static void pushbox_sidle_charmed(struct fmn_sprite *sprite) {
  switch (charmdir) {
    case FMN_DIR_N:
    case FMN_DIR_S: {
        int16_t mod=sprite->x%FMN_MM_PER_TILE;
        if (mod>=FMN_MM_PER_TILE>>1) sprite->x++;
        else if (mod) sprite->x--;
        else return;
      } break;
    case FMN_DIR_W:
    case FMN_DIR_E: {
        int16_t mod=sprite->y%FMN_MM_PER_TILE;
        if (mod>=FMN_MM_PER_TILE>>1) sprite->y++;
        else if (mod) sprite->y--;
        else return;
      } break;
    default: return;
  }
  fmn_sprite_collide(0,0,sprite,FMN_TILE_SOLID|FMN_TILE_HOLE,FMN_SPRITE_FLAG_SOLID,1);
}

static void pushbox_move_charmed(struct fmn_sprite *sprite) {

  // Confirm the hero's center is still in the ribbon stretching out from my charm direction.
  // If not, get unbewitched.
  int16_t dx=0,dy=0;
  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  switch (charmdir) {
    case FMN_DIR_N: {
        if ((herox<sprite->x)||(herox>=sprite->x+sprite->w)) {
          charmdir=0;
        } else {
          dy=-1;
        }
      } break;
    case FMN_DIR_S: {
        if ((herox<sprite->x)||(herox>=sprite->x+sprite->w)) {
          charmdir=0;
        } else {
          dy=1;
        }
      } break;
    case FMN_DIR_W: {
        if ((heroy<sprite->y)||(heroy>=sprite->y+sprite->h)) {
          charmdir=0;
        } else {
          dx=-1;
        }
      } break;
    case FMN_DIR_E: {
        if ((heroy<sprite->y)||(heroy>=sprite->y+sprite->h)) {
          charmdir=0;
        } else {
          dx=1;
        }
      } break;
    default: charmdir=0;
  }
  if (!charmdir) return;
  
  sprite->x+=dx*PUSHBOX_CHARM_SPEED;
  sprite->y+=dy*PUSHBOX_CHARM_SPEED;
  int16_t adjx,adjy;
  if (fmn_sprite_collide(&adjx,&adjy,sprite,FMN_TILE_SOLID|FMN_TILE_HOLE,FMN_SPRITE_FLAG_SOLID,1)) {
    if (!adjx&&!adjy) {
      sprite->x-=dx*PUSHBOX_CHARM_SPEED;
      sprite->y-=dy*PUSHBOX_CHARM_SPEED;
    } else {
      pushbox_sidle_charmed(sprite);
    }
  }
}

static void _pushbox_update(struct fmn_sprite *sprite) {

  // If we happen to get charmed during a move, do still complete the move first.
  if (movec&&(movex||movey)) {
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
  
  } else if (charmdir) {
    pushbox_move_charmed(sprite);
  }
}

static void _pushbox_push(struct fmn_sprite *sprite,int8_t dx,int8_t dy) {
  movex=dx*SPEED;
  movey=dy*SPEED;
  movec=FMN_MM_PER_TILE/SPEED;
}

static uint8_t _pushbox_featherspell(struct fmn_sprite *sprite,const uint8_t *v,uint8_t c) {
  // Our only feather spell is the simplest thing you can imagine:
  // One word, all are valid, and it triggers us to start moving in that direction until the hero walks away laterally.
  if (c<1) return 0;
  charmdir=v[c-1];
  return 1;
}

const struct fmn_sprtype fmn_sprtype_pushbox={
  .name="pushbox",
  .init=_pushbox_init,
  .update=_pushbox_update,
  .render=fmn_sprite_render_default,
  .push=_pushbox_push,
  .featherspell=_pushbox_featherspell,
};
