#include "game/fullmoon.h"
#include "game/model/fmn_map.h"
#include "game/fmn_data.h"
#include "game/fmn_play.h"
#include "game/fmn_pause.h"
#include "fmn_hero.h"

#define FMN_HERO_WALKSPEED_MAX 6
#define FMN_HERO_FLYSPEED_MAX  9

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
  fmn_hero.x+=fmn_hero.dx*fmn_hero.walkspeed;
  fmn_hero.y+=fmn_hero.dy*fmn_hero.walkspeed;
  if (fmn_hero.button&&(fmn_hero.action==FMN_ACTION_BROOM)) {
    if (fmn_hero.walkspeed<FMN_HERO_FLYSPEED_MAX) fmn_hero.walkspeed++;
  } else {
    if (fmn_hero.walkspeed<FMN_HERO_WALKSPEED_MAX) fmn_hero.walkspeed++;
    else if (fmn_hero.walkspeed>FMN_HERO_WALKSPEED_MAX) fmn_hero.walkspeed--;
  }
  
  //TODO collisions
  
  // Clamp hard to map boundaries. (TODO debatable. Will there ever be map neighbors that you can reach like screen neighbors?)
  int16_t mapwmm,maphmm;
  fmn_map_get_size_mm(&mapwmm,&maphmm);
  if (fmn_hero.x<0) fmn_hero.x=0;
  else if (fmn_hero.x>mapwmm-FMN_MM_PER_TILE) fmn_hero.x=mapwmm-FMN_MM_PER_TILE;
  if (fmn_hero.y<0) fmn_hero.y=0;
  else if (fmn_hero.y>maphmm-FMN_MM_PER_TILE) fmn_hero.y=maphmm-FMN_MM_PER_TILE;
  
  /* Actuate footswitches etc, and scroll to neighbor screens.
   * For these purposes, our position is a single point, at the center of the body tile.
   */
  int16_t x=fmn_hero.x+(FMN_MM_PER_TILE>>1);
  int16_t y=fmn_hero.y+(FMN_MM_PER_TILE>>1);
  if (fmn_game_focus_mm(x,y)) return;
  
  //TODO footswitches etc
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

static void fmn_hero_end_action() {
  switch (fmn_hero.action) {
    case FMN_ACTION_WAND: {
        if (!fmn_hero.spellc) return; // empty, just let it slide. they know it's not a valid spell.
        if ((fmn_hero.spellc<=FMN_SPELL_LENGTH_LIMIT)&&fmn_game_cast_spell(fmn_hero.spell,fmn_hero.spellc)) {
        } else {
          fmn_hero.spellrepudiation=63;
        }
      } break;
    case FMN_ACTION_UMBRELLA: {
        fmn_hero_update_facedir();
      } break;
  }
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

void fmn_hero_set_action(uint8_t action) {
  if (action==fmn_hero.action) return;
  if (fmn_hero.button) fmn_hero_end_action();
  fmn_hero.action=action;
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
    fmn_hero.button=0;
    fmn_hero_end_action();
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
    fmn_hero_update_action();
  }
}

/* Render riding broom.
 */
 
static void fmn_hero_render_broom(struct fmn_image *dst,int16_t dstx,int16_t dsty) {
  int16_t srcx=96;
  int16_t bodydsty=dsty-8;
  int16_t shadowsrcy=16;
  if (fmn_hero.facedir==FMN_DIR_W) {
    srcx=112;
  }
  if (fmn_hero.actionframe) {
    bodydsty++;
    shadowsrcy=24;
  }
  if (fmn_hero.actionparam&1) {
    fmn_blit(dst,dstx-6,dsty+4,&mainsprites,96,shadowsrcy,16,8);
  }
  fmn_blit(dst,dstx-6,bodydsty,&mainsprites,srcx,0,16,16);
}

/* Render using magic wand.
 */
 
static void fmn_hero_render_wand(struct fmn_image *dst,int16_t dstx,int16_t dsty) {
  uint8_t frame=0; // (0..4)=idle,left,right,down,up
  if (fmn_hero.spellrepudiation) {
    frame=5+((fmn_hero.spellrepudiation&8)?1:0);
    dstx-=4; dsty-=6;
  } else switch (fmn_hero.actionparam) {
    case 0: dstx-=4; dsty-=6; break;
    case FMN_DIR_W: frame=1; dstx-=8; dsty-=6; break;
    case FMN_DIR_E: frame=2; dsty-=6; break;
    case FMN_DIR_S: frame=3; dstx-=4; dsty-=6; break;
    case FMN_DIR_N: frame=4; dstx-=4; dsty-=8; break;
  }
  fmn_blit(dst,dstx,dsty,&mainsprites,frame*16,48,16,16);
}

/* Render arm.
 */
 
static void fmn_hero_render_arm(struct fmn_image *dst,int16_t dstx,int16_t dsty) {
  
  // Arm may render different when an action is in progress.
  if (fmn_hero.button) {
    switch (fmn_hero.action) {
    
      case FMN_ACTION_FEATHER: {
          uint8_t tileid=0x08;
          switch (fmn_hero.actionframe) {
            case 1: case 5: tileid+=0x10; break;
            case 2: case 4: tileid+=0x20; break;
            case 3: tileid+=0x30; break;
          }
          switch (fmn_hero.facedir) {
            case FMN_DIR_W: dstx-=6; dsty-=1; break;
            case FMN_DIR_E: tileid+=1; dstx+=6; dsty-=1; break;
            case FMN_DIR_N: tileid+=3; dsty-=6; break;
            case FMN_DIR_S: tileid+=2; dstx-=2; dsty+=5; break;
          }
          fmn_blit_tile(dst,dstx,dsty,&mainsprites,tileid);
        } return;
        
      case FMN_ACTION_UMBRELLA: {
          if (fmn_hero.actionframe==0) { // deploying
            int16_t d=(fmn_hero.actionanimtime*FMN_TILESIZE)/20;
            switch (fmn_hero.facedir) {
              case FMN_DIR_W: fmn_blit(dst,dstx-2-d,dsty,&mainsprites,64,32,16,8); break;
              case FMN_DIR_E: fmn_blit(dst,dstx-2+d,dsty,&mainsprites,64,40,16,8); break;
              case FMN_DIR_S: fmn_blit(dst,dstx-2,dsty-6+d,&mainsprites,80,32,8,16); break;
              case FMN_DIR_N: fmn_blit(dst,dstx+6-(d>>1),dsty-10-d,&mainsprites,88,32,8,16); break;
            }
          } else { // stable
            switch (fmn_hero.facedir) {
              case FMN_DIR_W: fmn_blit(dst,dstx-4,dsty-4,&mainsprites,96,32,8,16); break;
              case FMN_DIR_E: fmn_blit(dst,dstx+4,dsty-4,&mainsprites,104,32,8,16); break;
              case FMN_DIR_S: fmn_blit(dst,dstx-5,dsty+4,&mainsprites,112,40,16,8); break;
              case FMN_DIR_N: fmn_blit(dst,dstx-4,dsty-10,&mainsprites,112,32,16,8); break;
            }
          }
        } return;
    }
  }
  
  // General arm. North and west it's on the right. South and east on the left.
  if ((fmn_hero.facedir==FMN_DIR_N)||(fmn_hero.facedir==FMN_DIR_W)) {
    fmn_blit_tile(dst,dstx+6,dsty-1,&mainsprites,0x33+fmn_hero.action);
    fmn_blit_tile(dst,dstx+6,dsty-FMN_TILESIZE-1,&mainsprites,0x23+fmn_hero.action);
  } else {
    fmn_blit_tile(dst,dstx-6,dsty-1,&mainsprites,0x13+fmn_hero.action);
    fmn_blit_tile(dst,dstx-6,dsty-FMN_TILESIZE-1,&mainsprites,0x03+fmn_hero.action);
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
  
  // Arm before body+head, if facing north.
  if (fmn_hero.facedir==FMN_DIR_N) {
    fmn_hero_render_arm(dst,dstx,dsty);
  }
  
  // Head and body.
  uint8_t tilex=0;
  switch (fmn_hero.facedir) {
    case FMN_DIR_N: tilex=2; break;
    case FMN_DIR_W: tilex=1; break;
  }
  switch (fmn_hero.bodyframe) {
    case 0: case 2: fmn_blit_tile(dst,dstx,dsty,&mainsprites,0x10+tilex); break;
    case 1: fmn_blit_tile(dst,dstx,dsty,&mainsprites,0x20+tilex); break;
    case 3: fmn_blit_tile(dst,dstx,dsty,&mainsprites,0x30+tilex); break;
  }
  fmn_blit_tile(dst,dstx,dsty-6,&mainsprites,0x00+tilex);
  
  // Arm after body+head, for all but north.
  if (fmn_hero.facedir!=FMN_DIR_N) {
    fmn_hero_render_arm(dst,dstx,dsty);
  }
}

/* Get position.
 */
 
void fmn_hero_get_world_position(int16_t *xmm,int16_t *ymm) {
  *xmm=fmn_hero.x;
  *ymm=fmn_hero.y;
}

void fmn_hero_get_screen_position(int16_t *xpx,int16_t *ypx) {
  uint8_t scrollx,scrolly;
  fmn_map_get_scroll(&scrollx,&scrolly);
  *xpx=(fmn_hero.x-(scrollx*FMN_MM_PER_TILE))/FMN_MM_PER_PIXEL;
  *ypx=(fmn_hero.y-(scrolly*FMN_MM_PER_TILE))/FMN_MM_PER_PIXEL;
}