#include "game/fullmoon.h"
#include "game/model/fmn_map.h"
#include "game/fmn_data.h"
#include "game/fmn_play.h"
#include "game/fmn_pause.h"
#include "game/model/fmn_hero.h"
#include "game/sprite/fmn_sprite.h"

#define FMN_HERO_WALKSPEED_MAX 6
#define FMN_HERO_FLYSPEED_MAX  9

// Hitbox in mm relative to canonical position (near left shoulder).
// Beware these must match src/game/sprite/type/fmn_sprite_heroproxy.c
#define FMN_HERO_HITBOX_X 8
#define FMN_HERO_HITBOX_Y 8
#define FMN_HERO_HITBOX_W (FMN_MM_PER_TILE-16)
#define FMN_HERO_HITBOX_H (FMN_MM_PER_TILE-16)

/* Globals.
 */
 
static struct fmn_hero fmn_hero={0};

/* Reset.
 */
 
void fmn_hero_reset() {
  uint8_t col,row;
  fmn_map_get_init_position(&col,&row);
  fmn_hero.x=col*FMN_MM_PER_TILE;
  fmn_hero.y=row*FMN_MM_PER_TILE;
  fmn_hero.dx=0;
  fmn_hero.dy=0;
  fmn_hero.button=0;
  fmn_hero.walkspeed=0;
  fmn_hero.action=fmn_pause_get_action();
  fmn_hero.facedir=FMN_DIR_S;
  fmn_hero.actionframe=0;
  fmn_hero.actionanimtime=0;
  fmn_hero.actionparam=0;
  fmn_hero.spellc=0;
  fmn_hero.spellrepudiation=0;
}

/* Motion.
 */
 
static void fmn_hero_begin_motion(int8_t dx,int8_t dy) {
}

static void fmn_hero_end_motion() {
  fmn_hero.walkspeed=0;
}
 
static void fmn_hero_update_motion() {

  // Apply motion optimistically.
  int16_t pvx=fmn_hero.x,pvy=fmn_hero.y;
  fmn_hero.x+=fmn_hero.dx*fmn_hero.walkspeed;
  fmn_hero.y+=fmn_hero.dy*fmn_hero.walkspeed;
  if (fmn_hero.button&&(fmn_hero.action==FMN_ACTION_BROOM)) {
    if (fmn_hero.walkspeed<FMN_HERO_FLYSPEED_MAX) fmn_hero.walkspeed++;
  } else {
    if (fmn_hero.walkspeed<FMN_HERO_WALKSPEED_MAX) fmn_hero.walkspeed++;
    else if (fmn_hero.walkspeed>FMN_HERO_WALKSPEED_MAX) fmn_hero.walkspeed--;
  }
  
  // Resolve collisions.
  uint8_t solids=FMN_TILE_SOLID;
  if (fmn_hero.button&&(fmn_hero.action==FMN_ACTION_BROOM)) ;
  else solids|=FMN_TILE_HOLE;
  int16_t adjx=0,adjy=0;
  if (fmn_map_check_collision(&adjx,&adjy,
    fmn_hero.x+FMN_HERO_HITBOX_X,
    fmn_hero.y+FMN_HERO_HITBOX_Y,
    FMN_HERO_HITBOX_W,
    FMN_HERO_HITBOX_H,
    solids,
    FMN_SPRITE_FLAG_SOLID
  )) {
    if (adjx||adjy) {
      fmn_hero.x+=adjx;
      fmn_hero.y+=adjy;
    } else {
      fmn_hero.x=pvx;
      fmn_hero.y=pvy;
    }
  }
  
  /* Scroll to neighbor screens or thru edge doors.
   * For these purposes, our position is a single point, at the center of the body tile.
   * If the screen changes, remeasure. We might have moved to another map.
   */
  int16_t x=fmn_hero.x+(FMN_MM_PER_TILE>>1);
  int16_t y=fmn_hero.y+(FMN_MM_PER_TILE>>1);
  if (fmn_game_focus_mm(x,y)) {
    x=fmn_hero.x+(FMN_MM_PER_TILE>>1);
    y=fmn_hero.y+(FMN_MM_PER_TILE>>1);
  }
  
  /* Check treadle POIs if we change cells.
   */
  int16_t cellx=x/FMN_MM_PER_TILE; if (x<0) cellx--;
  int16_t celly=y/FMN_MM_PER_TILE; if (y<0) celly--;
  if ((cellx!=fmn_hero.cellx)||(celly!=fmn_hero.celly)) {
    fmn_map_exit_cell(fmn_hero.cellx,fmn_hero.celly);
    fmn_hero.cellx=cellx;
    fmn_hero.celly=celly;
    if (fmn_map_enter_cell(fmn_hero.cellx,fmn_hero.celly)) {
      // Screen changed. If we're going to stay here, we must update position.
      return;
    }
  }
}

/* Update facedir from (dx,dy). And possibly other things.
 */
 
static void fmn_hero_update_facedir() {

  // If umbrella in use, no change of direction.
  if (fmn_hero.button&&(fmn_hero.action==FMN_ACTION_UMBRELLA)) return;
  
  // If broom in use, direction can only be horizontal.
  if (fmn_hero.button&&(fmn_hero.action==FMN_ACTION_BROOM)) {
    if (fmn_hero.dx<0) fmn_hero.facedir=FMN_DIR_W;
    else if (fmn_hero.dx>0) fmn_hero.facedir=FMN_DIR_E;
    else if ((fmn_hero.facedir!=FMN_DIR_W)&&(fmn_hero.facedir!=FMN_DIR_E)) {
      fmn_hero.facedir=FMN_DIR_E;
    }
    return;
  }

  if (fmn_hero.dx<0) {
    fmn_hero.facedir=FMN_DIR_W;
  } else if (fmn_hero.dx>0) {
    fmn_hero.facedir=FMN_DIR_E;
  } else if (fmn_hero.dy<0) {
    fmn_hero.facedir=FMN_DIR_N;
  } else if (fmn_hero.dy>0) {
    fmn_hero.facedir=FMN_DIR_S;
  }
}

/* Actions.
 */

static void fmn_hero_begin_action() {
  switch (fmn_hero.action) {
    case FMN_ACTION_BROOM: {
        fmn_hero_update_facedir();
        fmn_hero.actionframe=0;
        fmn_hero.actionanimtime=0;
      } break;
    case FMN_ACTION_FEATHER: {
        fmn_hero.actionframe=0;
        fmn_hero.actionanimtime=0;
      } break;
    case FMN_ACTION_WAND: {
        if (fmn_hero.dx||fmn_hero.dy) {
          fmn_hero_end_motion();
          fmn_hero.dx=0;
          fmn_hero.dy=0;
        }
        fmn_hero.actionparam=0;
        fmn_hero.spellc=0;
      } break;
    case FMN_ACTION_UMBRELLA: {
        fmn_hero.actionframe=0;
        fmn_hero.actionanimtime=0;
      } break;
  }
}

// Nonzero if ended. Some are conditional eg you can't stop flying over a hole.
static uint8_t fmn_hero_end_action() {
  switch (fmn_hero.action) {
    case FMN_ACTION_BROOM: {
        // Reject if offscreen or over a hole.
        if (fmn_hero.cellx<0) return 0;
        if (fmn_hero.celly<0) return 0;
        uint8_t mapw,maph;
        fmn_map_get_size(&mapw,&maph);
        if (fmn_hero.cellx>=mapw) return 0;
        if (fmn_hero.celly>=maph) return 0;
        if (fmn_map_check_collision(0,0,
          fmn_hero.x+FMN_HERO_HITBOX_X,
          fmn_hero.y+FMN_HERO_HITBOX_Y,
          FMN_HERO_HITBOX_W,
          FMN_HERO_HITBOX_H,
          FMN_TILE_HOLE,
          FMN_SPRITE_FLAG_SOLID
        )) return 0;
      } break;
    case FMN_ACTION_WAND: {
        if (!fmn_hero.spellc) return 1; // empty, just let it slide. they know it's not a valid spell.
        if ((fmn_hero.spellc<=FMN_SPELL_LENGTH_LIMIT)&&fmn_game_cast_spell(fmn_hero.spell,fmn_hero.spellc)) {
        } else {
          fmn_hero.spellrepudiation=63;
        }
      } break;
    case FMN_ACTION_UMBRELLA: {
        fmn_hero_update_facedir();
      } break;
  }
  fmn_hero.end_action_when_possible=0;
  return 1;
}
 
static void fmn_hero_update_action() {
  switch (fmn_hero.action) {
    case FMN_ACTION_BROOM: {
        if (fmn_hero.actionanimtime) fmn_hero.actionanimtime--;
        else {
          fmn_hero.actionanimtime=8;
          fmn_hero.actionframe^=1;
        }
        fmn_hero.actionparam++;
      } break;
    case FMN_ACTION_FEATHER: {
        if (fmn_hero.actionanimtime) fmn_hero.actionanimtime--;
        else {
          fmn_hero.actionanimtime=5;
          if (++(fmn_hero.actionframe)>=6) fmn_hero.actionframe=0;
        }
        //TODO actuate things with the feather
      } break;
    case FMN_ACTION_WAND: {
      } break;
    case FMN_ACTION_UMBRELLA: {
        // Umbrella: "frame zero" means deploying, "frame one" means stable.
        if (fmn_hero.actionframe==0) {
          fmn_hero.actionanimtime++;
          if (fmn_hero.actionanimtime>=20) {
            fmn_hero.actionframe=1;
          }
        }
      } break;
  }
}

uint8_t fmn_hero_set_action(uint8_t action) {
  if (action==fmn_hero.action) return 1;
  if (fmn_hero.button) {
    if (!fmn_hero_end_action()) return 0;
  }
  fmn_hero.action=action;
  return 1;
}

/* Set inputs.
 */
 
void fmn_hero_set_input(int8_t dx,int8_t dy,uint8_t button) {

  if (dx||dy) {
    if (fmn_hero.button&&(fmn_hero.action==FMN_ACTION_WAND)) {
      int16_t param=fmn_hero.actionparam;
      if ((dx<0)&&(dy==0)) param=FMN_DIR_W;
      else if ((dx>0)&&(dy==0)) param=FMN_DIR_E;
      else if ((dx==0)&&(dy<0)) param=FMN_DIR_N;
      else if ((dx==0)&&(dy>0)) param=FMN_DIR_S;
      if (param!=fmn_hero.actionparam) {
        if (param) {
          if (fmn_hero.spellc<FMN_SPELL_LENGTH_LIMIT) fmn_hero.spell[fmn_hero.spellc]=param;
          if (fmn_hero.spellc<100) fmn_hero.spellc++; // stop incrementing at some point
        }
        fmn_hero.actionparam=param;
      }
    } else {
      if (!fmn_hero.dx&&!fmn_hero.dy) fmn_hero_begin_motion(dx,dy);
      fmn_hero.dx=dx;
      fmn_hero.dy=dy;
      fmn_hero_update_facedir();
    }
    
  } else if (fmn_hero.button&&(fmn_hero.action==FMN_ACTION_WAND)) {
    fmn_hero.actionparam=0;
    
  } else if (fmn_hero.dx||fmn_hero.dy) {
    fmn_hero_end_motion();
    fmn_hero.dx=0;
    fmn_hero.dy=0;
  }

  if (button&&!fmn_hero.button) {
    fmn_hero.button=1;
    fmn_hero_begin_action();
  } else if (!button&&fmn_hero.button) {
    if (fmn_hero_end_action()) {
      fmn_hero.button=0;
    } else {
      fmn_hero.end_action_when_possible=1;
    }
  }
}

/* Update animation.
 */
 
static void fmn_hero_update_animation() {

  if (fmn_hero.spellrepudiation) fmn_hero.spellrepudiation--;
  
  if (fmn_hero.dx||fmn_hero.dy) {
    if (fmn_hero.bodyanimtime) {
      fmn_hero.bodyanimtime--;
    } else {
      fmn_hero.bodyanimtime=6;
      if (++(fmn_hero.bodyframe)>=4) {
        fmn_hero.bodyframe=0;
      }
    }
  } else {
    fmn_hero.bodyframe=0;
    fmn_hero.bodyanimtime=0;
  }
  
}

/* Update.
 */
 
void fmn_hero_update() {
  fmn_hero_update_animation();
  if (fmn_hero.dx||fmn_hero.dy) {
    fmn_hero_update_motion();
  }
  if (fmn_hero.button) {
    if (fmn_hero.end_action_when_possible&&fmn_hero_end_action()) {
      fmn_hero.button=0;
    } else {
      fmn_hero_update_action();
    }
  }
}

/* Render riding broom.
 */
 
static void fmn_hero_render_broom(struct fmn_image *dst,int16_t dstx,int16_t dsty) {
  uint8_t xform=0;
  int16_t bodydsty=dsty-8*FMN_GFXSCALE;
  int16_t shadowsrcy=16*FMN_GFXSCALE;
  if (fmn_hero.facedir==FMN_DIR_W) {
    xform=FMN_XFORM_XREV;
  }
  if (fmn_hero.actionframe) {
    bodydsty+=FMN_GFXSCALE;
    shadowsrcy=24*FMN_GFXSCALE;
  }
  if (fmn_hero.actionparam&1) {
    fmn_blit(dst,dstx-6*FMN_GFXSCALE,dsty+4*FMN_GFXSCALE,&mainsprites,40*FMN_GFXSCALE,shadowsrcy,FMN_NSCOORD(16,8),0);
  }
  fmn_blit(dst,dstx-6*FMN_GFXSCALE,bodydsty,&mainsprites,FMN_NSCOORD(24,16),FMN_NSCOORD(16,16),xform);
}

/* Render using magic wand.
 */
 
static void fmn_hero_render_wand(struct fmn_image *dst,int16_t dstx,int16_t dsty) {
  uint8_t frame=0; // (0..3)=idle,left,down,up
  uint8_t xform=0;
  if (fmn_hero.spellrepudiation) {
    frame=4+((fmn_hero.spellrepudiation&8)?1:0);
    dstx-=4*FMN_GFXSCALE; dsty-=6*FMN_GFXSCALE;
  } else switch (fmn_hero.actionparam) {
    case 0: dstx-=4*FMN_GFXSCALE; dsty-=6*FMN_GFXSCALE; break;
    case FMN_DIR_W: frame=1; dstx-=8*FMN_GFXSCALE; dsty-=6*FMN_GFXSCALE; break;
    case FMN_DIR_E: frame=1; dsty-=6*FMN_GFXSCALE; xform=FMN_XFORM_XREV; break;
    case FMN_DIR_S: frame=2; dstx-=4*FMN_GFXSCALE; dsty-=6*FMN_GFXSCALE; break;
    case FMN_DIR_N: frame=3; dstx-=4*FMN_GFXSCALE; dsty-=8*FMN_GFXSCALE; break;
  }
  fmn_blit(dst,dstx,dsty,&mainsprites,FMN_NSCOORD(frame*16,48),FMN_NSCOORD(16,16),xform);
}

/* Render arm.
 */
 
static void fmn_hero_render_arm(struct fmn_image *dst,int16_t dstx,int16_t dsty) {
  
  // Arm may render different when an action is in progress.
  if (fmn_hero.button) {
    switch (fmn_hero.action) {
    
      case FMN_ACTION_FEATHER: {
          uint8_t tileid=0x01,xform=0;
          switch (fmn_hero.actionframe) {
            case 1: case 5: tileid+=0x10; break;
            case 2: case 4: tileid+=0x20; break;
            case 3: tileid+=0x30; break;
          }
          switch (fmn_hero.facedir) {
            case FMN_DIR_W: dstx-=6*FMN_GFXSCALE; dsty-=1*FMN_GFXSCALE; break;
            case FMN_DIR_E: dstx+=6*FMN_GFXSCALE; dsty-=1*FMN_GFXSCALE; xform=FMN_XFORM_XREV; break;
            case FMN_DIR_N: dsty-=6*FMN_GFXSCALE; xform=FMN_XFORM_SWAP|FMN_XFORM_YREV; break;
            case FMN_DIR_S: dstx-=2*FMN_GFXSCALE; dsty+=4*FMN_GFXSCALE; xform=FMN_XFORM_SWAP|FMN_XFORM_XREV; break;
          }
          fmn_blit_tile(dst,dstx,dsty,&mainsprites,tileid,xform);
        } return;
        
      case FMN_ACTION_UMBRELLA: {
          if (fmn_hero.actionframe==0) { // deploying
            int16_t d=(fmn_hero.actionanimtime*FMN_TILESIZE)/20;
            switch (fmn_hero.facedir) {
              case FMN_DIR_W: fmn_blit(dst,dstx-2*FMN_GFXSCALE-d,dsty,&mainsprites,FMN_NSCOORD(64,32),FMN_NSCOORD(8,16),FMN_XFORM_SWAP|FMN_XFORM_XREV); break;
              case FMN_DIR_E: fmn_blit(dst,dstx-2*FMN_GFXSCALE+d,dsty,&mainsprites,FMN_NSCOORD(64,32),FMN_NSCOORD(8,16),FMN_XFORM_SWAP|FMN_XFORM_YREV); break;
              case FMN_DIR_S: fmn_blit(dst,dstx-2*FMN_GFXSCALE,dsty-6*FMN_GFXSCALE+d,&mainsprites,FMN_NSCOORD(64,32),FMN_NSCOORD(8,16),FMN_XFORM_YREV); break;
              case FMN_DIR_N: fmn_blit(dst,dstx+6*FMN_GFXSCALE-(d>>1),dsty-10*FMN_GFXSCALE-d,&mainsprites,FMN_NSCOORD(72,32),FMN_NSCOORD(8,16),0); break;
            }
          } else { // stable
            switch (fmn_hero.facedir) {
              case FMN_DIR_W: fmn_blit(dst,dstx-4*FMN_GFXSCALE,dsty-4*FMN_GFXSCALE,&mainsprites,FMN_NSCOORD(80,32),FMN_NSCOORD(8,16),0); break;
              case FMN_DIR_E: fmn_blit(dst,dstx+4*FMN_GFXSCALE,dsty-4*FMN_GFXSCALE,&mainsprites,FMN_NSCOORD(80,32),FMN_NSCOORD(8,16),FMN_XFORM_XREV); break;
              case FMN_DIR_S: fmn_blit(dst,dstx-5*FMN_GFXSCALE,dsty+4*FMN_GFXSCALE,&mainsprites,FMN_NSCOORD(80,32),FMN_NSCOORD(8,16),FMN_XFORM_SWAP|FMN_XFORM_XREV); break;
              case FMN_DIR_N: fmn_blit(dst,dstx-4*FMN_GFXSCALE,dsty-10*FMN_GFXSCALE,&mainsprites,FMN_NSCOORD(80,32),FMN_NSCOORD(8,16),FMN_XFORM_SWAP); break;
            }
          }
        } return;
    }
  }
  
  // General arm. North and west it's on the right. South and east on the left.
  if ((fmn_hero.facedir==FMN_DIR_N)||(fmn_hero.facedir==FMN_DIR_W)) {
    uint8_t tileid=0x03+fmn_hero.action;
    if (fmn_hero.facedir==FMN_DIR_N) tileid+=4;
    fmn_blit_tile(dst,dstx+6*FMN_GFXSCALE,dsty-FMN_GFXSCALE,&mainsprites,tileid+0x10,FMN_XFORM_XREV);
    fmn_blit_tile(dst,dstx+6*FMN_GFXSCALE,dsty-FMN_TILESIZE-FMN_GFXSCALE,&mainsprites,tileid,FMN_XFORM_XREV);
  } else {
    fmn_blit_tile(dst,dstx-6*FMN_GFXSCALE,dsty-FMN_GFXSCALE,&mainsprites,0x13+fmn_hero.action,0);
    fmn_blit_tile(dst,dstx-6*FMN_GFXSCALE,dsty-FMN_TILESIZE-FMN_GFXSCALE,&mainsprites,0x03+fmn_hero.action,0);
  }
}

/* Render.
 */
 
void fmn_hero_render(struct fmn_image *dst) {
  int16_t dstx,dsty;
  fmn_hero_get_screen_position(&dstx,&dsty);
  
  // Some actions change the rendering completely.
  if (fmn_hero.button) switch (fmn_hero.action) {
    case FMN_ACTION_BROOM: fmn_hero_render_broom(dst,dstx,dsty); return;
    case FMN_ACTION_WAND: fmn_hero_render_wand(dst,dstx,dsty); return;
  }
  if (fmn_hero.spellrepudiation) {
    fmn_hero_render_wand(dst,dstx,dsty);
    return;
  }
  
  // Arm before body+head, if facing north or using feather.
  uint8_t didarm=0;
  if (
    (fmn_hero.facedir==FMN_DIR_N)||
    ((fmn_hero.action==FMN_ACTION_FEATHER)&&fmn_hero.button&&(fmn_hero.facedir!=FMN_DIR_S))
  ) {
    didarm=1;
    fmn_hero_render_arm(dst,dstx,dsty);
  }
  
  // Head and body.
  uint8_t tilex=0,xform=0;
  switch (fmn_hero.facedir) {
    case FMN_DIR_N: tilex=2; break;
    case FMN_DIR_W: xform=FMN_XFORM_XREV; break;
  }
  switch (fmn_hero.bodyframe) {
    case 0: case 2: fmn_blit_tile(dst,dstx,dsty,&mainsprites,0x10+tilex,xform); break;
    case 1: fmn_blit_tile(dst,dstx,dsty,&mainsprites,0x20+tilex,xform); break;
    case 3: fmn_blit_tile(dst,dstx,dsty,&mainsprites,0x30+tilex,xform); break;
  }
  fmn_blit_tile(dst,dstx,dsty-6*FMN_GFXSCALE,&mainsprites,0x00+tilex,xform);
  
  // Arm after body+head, if we didn't already draw it.
  if (!didarm) {
    fmn_hero_render_arm(dst,dstx,dsty);
  }
  
}

/* Get position.
 */
 
void fmn_hero_get_world_position(int16_t *xmm,int16_t *ymm) {
  *xmm=fmn_hero.x;
  *ymm=fmn_hero.y;
}

void fmn_hero_get_world_position_center(int16_t *xmm,int16_t *ymm) {
  *xmm=fmn_hero.x+(FMN_MM_PER_TILE>>1);
  *ymm=fmn_hero.y+(FMN_MM_PER_TILE>>1);
}

void fmn_hero_get_outer_bounds(int16_t *xmm,int16_t *ymm,int16_t *w,int16_t *h) {
  // This is not a perfect fit. There's some waste most of the time, and the umbrella escapes it while deploying.
  // I think it's close enough.
  *xmm=fmn_hero.x-FMN_MM_PER_TILE;
  *ymm=fmn_hero.y-FMN_MM_PER_TILE-(FMN_MM_PER_TILE>>1);
  *w=FMN_MM_PER_TILE*3;
  *h=FMN_MM_PER_TILE*3;
}

void fmn_hero_get_screen_position(int16_t *xpx,int16_t *ypx) {
  uint8_t scrollx,scrolly;
  fmn_map_get_scroll(&scrollx,&scrolly);
  *xpx=((fmn_hero.x-(scrollx*FMN_MM_PER_TILE))*FMN_TILESIZE)/FMN_MM_PER_TILE;
  *ypx=((fmn_hero.y-(scrolly*FMN_MM_PER_TILE))*FMN_TILESIZE)/FMN_MM_PER_TILE;
}

/* Force position eg on a map change.
 */
 
void fmn_hero_force_position(int16_t xmm,int16_t ymm) {
  fmn_hero.x=xmm;
  fmn_hero.y=ymm;
  fmn_hero.cellx=(xmm+(FMN_MM_PER_TILE>>1))/FMN_MM_PER_TILE; if (xmm<0) fmn_hero.cellx--;
  fmn_hero.celly=(ymm+(FMN_MM_PER_TILE>>1))/FMN_MM_PER_TILE; if (ymm<0) fmn_hero.celly--;
}
