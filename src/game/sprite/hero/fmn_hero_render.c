#include "game/sprite/hero/fmn_hero_internal.h"
#include "game/fmn_image.h"
#include "game/fmn_data.h"

/* Update facedir based on (indx,indy).
 */
 
void fmn_hero_update_facedir() {

  // If umbrella is in use, do not change.
  if (fmn_hero.action_in_progress==FMN_ACTION_UMBRELLA) {
    return;
  }
  
  // If we are actively encoding with the wand, do not change.
  if (fmn_hero.action_in_progress==FMN_ACTION_WAND) {
    return;
  }

  // Change to agree with input state.
  // Prefer showing a horizontal face; they are prettier.
  #if 0 // ...actually I want to always face the direction of most recent motion, to aid with triggering the firewall.
  if (fmn_hero.indx<0) {
    fmn_hero.facedir=FMN_DIR_W;
    fmn_hero.xfacedir=FMN_DIR_W;
  } else if (fmn_hero.indx>0) {
    fmn_hero.facedir=FMN_DIR_E;
    fmn_hero.xfacedir=FMN_DIR_E;
  } else if (fmn_hero.indy<0) {
    fmn_hero.facedir=FMN_DIR_N;
  } else if (fmn_hero.indy>0) {
    fmn_hero.facedir=FMN_DIR_S;
  }
  #else
  switch (fmn_hero.last_motion_dir) {
    case FMN_DIR_S: {
        if (fmn_hero.indy>0) fmn_hero.facedir=FMN_DIR_S;
        else if (fmn_hero.indx<0) fmn_hero.facedir=FMN_DIR_W;
        else if (fmn_hero.indx>0) fmn_hero.facedir=FMN_DIR_E;
      } break;
    case FMN_DIR_N: {
        if (fmn_hero.indy<0) fmn_hero.facedir=FMN_DIR_N;
        else if (fmn_hero.indx<0) fmn_hero.facedir=FMN_DIR_W;
        else if (fmn_hero.indx>0) fmn_hero.facedir=FMN_DIR_E;
      } break;
    case FMN_DIR_W: {
        if (fmn_hero.indx<0) fmn_hero.facedir=FMN_DIR_W;
        else if (fmn_hero.indy<0) fmn_hero.facedir=FMN_DIR_N;
        else if (fmn_hero.indy>0) fmn_hero.facedir=FMN_DIR_S;
      } break;
    case FMN_DIR_E: {
        if (fmn_hero.indx>0) fmn_hero.facedir=FMN_DIR_E;
        else if (fmn_hero.indy<0) fmn_hero.facedir=FMN_DIR_N;
        else if (fmn_hero.indy>0) fmn_hero.facedir=FMN_DIR_S;
      } break;
  }
  if (fmn_hero.facedir&(FMN_DIR_W|FMN_DIR_E)) {
    fmn_hero.xfacedir=fmn_hero.facedir&(FMN_DIR_W|FMN_DIR_E);
  }
  #endif
}

/* Some conveniences for rendering.
 */
 
// Provide coordinates in normalized relative pixels (ie assume 8x8 tiles).
#define TILE(rx,ry,tileid,xform) fmn_blit_tile(dst,dstx+(rx)*FMN_GFXSCALE,dsty+(ry)*FMN_GFXSCALE,&mainsprites,tileid,xform);
#define DECAL(rx,ry,srcx,srcy,w,h,xform) { \
  fmn_blit(dst,dstx+(rx)*FMN_GFXSCALE,dsty+(ry)*FMN_GFXSCALE,&mainsprites,FMN_NSCOORD(srcx,srcy),FMN_NSCOORD(w,h),xform); \
}

// Same idea but (rx,ry) are in real pixels. In case you are doing something that can scale continuously.
#define TILEPX(rx,ry,tileid,xform) fmn_blit_tile(dst,dstx+(rx),dsty+(ry),&mainsprites,tileid,xform);
#define DECALPX(rx,ry,srcx,srcy,w,h,xform) { \
  fmn_blit(dst,dstx+(rx),dsty+(ry),&mainsprites,FMN_NSCOORD(srcx,srcy),FMN_NSCOORD(w,h),xform); \
}

/* Flying on the broom.
 */
 
static void fmn_hero_render_broom(struct fmn_image *dst,int16_t dstx,int16_t dsty) {
  uint8_t phase=fmn_hero.framec&31;

  if (fmn_hero.framec&1) { // Shadow.
    int16_t srcy=16;
    if ((phase>=16)&&(phase<48)) srcy+=8;
    DECAL(-5,2,40,srcy,16,8,0)
  }
  
  uint8_t xform=(fmn_hero.xfacedir==FMN_DIR_W)?FMN_XFORM_XREV:0;
  int16_t addy;
  if (phase>=16) addy=31-phase;
  else addy=phase;
  addy=(addy*FMN_GFXSCALE*2)/16;
  DECALPX(-5*FMN_GFXSCALE,-11*FMN_GFXSCALE+addy,24,16,16,16,xform)
}

/* Encoding with the wand.
 */
 
static void fmn_hero_render_wand(struct fmn_image *dst,int16_t dstx,int16_t dsty) {
  int16_t srcx,addx=0,addy=0;
  uint8_t xform=0;
  if (fmn_hero.spellrepudiation) {
    srcx=64;
    if (fmn_hero.spellrepudiation&16) srcx+=16;
  } else if ((fmn_hero.indx<0)&&!fmn_hero.indy) {
    srcx=16;
    addx=-4;
  } else if ((fmn_hero.indx>0)&&!fmn_hero.indy) {
    srcx=16;
    addx=4;
    xform=FMN_XFORM_XREV;
  } else if (!fmn_hero.indx&&(fmn_hero.indy<0)) {
    srcx=48;
    addy=-2;
  } else if (!fmn_hero.indx&&(fmn_hero.indy>0)) {
    srcx=32;
  } else {
    srcx=0;
  }
  DECAL(-5+addx,-8+addy,srcx,48,16,16,xform)
}

/* Feather in progress.
 */
 
static void fmn_hero_render_arm_feather(
  struct fmn_image *dst,int16_t dstx,int16_t dsty
) {
  int16_t addx=0,addy=0;
  uint8_t tileid=0x01;
  tileid+=((fmn_hero.framec%32)/8)*0x10;
  uint8_t xform=0;
  switch (fmn_hero.facedir) {
    case FMN_DIR_W: addx=-8; addy=-3; xform=0; break;
    case FMN_DIR_E: addx=6; addy=-3; xform=FMN_XFORM_XREV; break;
    case FMN_DIR_N: addx=1; addy=-9; xform=FMN_XFORM_SWAP|FMN_XFORM_YREV; break;
    case FMN_DIR_S: addx=-2; addy=3; xform=FMN_XFORM_SWAP|FMN_XFORM_XREV; break;
  }
  TILE(addx,addy,tileid,xform)
}

/* Umbrella in progress.
 */
 
static void fmn_hero_render_arm_umbrella(
  struct fmn_image *dst,int16_t dstx,int16_t dsty
) {
  int16_t srcx=64,srcy=32;
  int16_t addx=0,addy=0;
  uint8_t xform=0;
  
  // Deployed.
  if (fmn_hero.umbrellatime>=FMN_HERO_UMBRELLA_TIME) {
    srcx+=16;
    switch (fmn_hero.facedir) {
      case FMN_DIR_W: {
          addx=-8*FMN_GFXSCALE;
          addy=-7*FMN_GFXSCALE;
        } break;
      case FMN_DIR_E: {
          xform=FMN_XFORM_XREV;
          addx=6*FMN_GFXSCALE;
          addy=-7*FMN_GFXSCALE;
        } break;
      case FMN_DIR_S: {
          xform=FMN_XFORM_SWAP|FMN_XFORM_XREV;
          addx=-4*FMN_GFXSCALE;
          addy=2*FMN_GFXSCALE;
        } break;
      case FMN_DIR_N: {
          xform=FMN_XFORM_SWAP;
          addx=-4*FMN_GFXSCALE;
          addy=-10*FMN_GFXSCALE;
        } break;
    }
  
  // Deploying. The tip of the umbrella should exceed its deployed position by 4 normalized pixels.
  } else switch (fmn_hero.facedir) {
    case FMN_DIR_W: {
        xform=FMN_XFORM_SWAP;
        addx=-6*FMN_GFXSCALE;
        addy=-1*FMN_GFXSCALE;
        addx+=(-6*FMN_GFXSCALE*fmn_hero.umbrellatime)/FMN_HERO_UMBRELLA_TIME;
        addy+=(-2*FMN_GFXSCALE*fmn_hero.umbrellatime)/FMN_HERO_UMBRELLA_TIME;
      } break;
    case FMN_DIR_E: {
        xform=FMN_XFORM_SWAP|FMN_XFORM_YREV;
        addx=-3*FMN_GFXSCALE;
        addy=-1*FMN_GFXSCALE;
        addx+=(5*FMN_GFXSCALE*fmn_hero.umbrellatime)/FMN_HERO_UMBRELLA_TIME;
        addy+=(-2*FMN_GFXSCALE*fmn_hero.umbrellatime)/FMN_HERO_UMBRELLA_TIME;
      } break;
    case FMN_DIR_S: {
        xform=FMN_XFORM_YREV;
        addx=-3*FMN_GFXSCALE;
        addy=-5*FMN_GFXSCALE;
        addx+=(3*FMN_GFXSCALE*fmn_hero.umbrellatime)/FMN_HERO_UMBRELLA_TIME;
        addy+=(3*FMN_GFXSCALE*fmn_hero.umbrellatime)/FMN_HERO_UMBRELLA_TIME;
      } break;
    case FMN_DIR_N: {
        srcx+=8;
        addx=5*FMN_GFXSCALE;
        addy=-11*FMN_GFXSCALE;
        addx+=(-3*FMN_GFXSCALE*fmn_hero.umbrellatime)/FMN_HERO_UMBRELLA_TIME;
        addy+=(-6*FMN_GFXSCALE*fmn_hero.umbrellatime)/FMN_HERO_UMBRELLA_TIME;
      } break;
  }
  DECALPX(addx,addy,srcx,srcy,8,16,xform)
}

/* Arms.
 */
 
static void fmn_hero_render_arm(
  struct fmn_image *dst,int16_t dstx,int16_t dsty,
  int8_t side, // -1=left, 1=right (*your* left and right)
  uint8_t hold_item
) {
  // Feather and Umbrella do something different for the "hold" hand, when in progress.
  if (hold_item) switch (fmn_hero.action_in_progress) {
    case FMN_ACTION_FEATHER: fmn_hero_render_arm_feather(dst,dstx,dsty); return;
    case FMN_ACTION_UMBRELLA: fmn_hero_render_arm_umbrella(dst,dstx,dsty); return;
  }
  int16_t addx=0;
  int16_t addy=-11;
  int16_t srcx=24;
  int16_t srcy=0;
  uint8_t xform=0;
  // Facing north, use the back-handed set.
  if (fmn_hero.facedir==FMN_DIR_N) {
    srcx+=40;
  }
  if (hold_item) {
    srcx+=8*fmn_hero.action;
  }
  // Left.
  if (side<0) {
    addx=-7;
  // Right.
  } else {
    addx=5;
    xform=FMN_XFORM_XREV;
  }
  DECAL(addx,addy,srcx,srcy,8,16,xform)
}

/* Body.
 */
 
static void fmn_hero_render_body(struct fmn_image *dst,int16_t dstx,int16_t dsty) {
  uint8_t tileid=0x10;
  uint8_t xform=0;
  switch (fmn_hero.facedir) {
    case FMN_DIR_N: tileid+=0x02; break;
    case FMN_DIR_W: xform=FMN_XFORM_XREV; break;
  }
  if (fmn_hero.indx||fmn_hero.indy) {
    fmn_hero.walkframeclock++;
    if (fmn_hero.walkframeclock>=6) {
      fmn_hero.walkframeclock=0;
      fmn_hero.walkframe++;
      if (fmn_hero.walkframe>=4) {
        fmn_hero.walkframe=0;
      }
    }
    switch (fmn_hero.walkframe) {
      case 1: case 3: break;
      case 0: tileid+=0x10; break;
      case 2: tileid+=0x20; break;
    }
  }
  TILE(-1,-2,tileid,xform)
}

/* Head.
 */
 
static void fmn_hero_render_head(struct fmn_image *dst,int16_t dstx,int16_t dsty) {
  uint8_t tileid=0x00;
  uint8_t xform=0;
  switch (fmn_hero.facedir) {
    case FMN_DIR_N: tileid+=0x02; break;
    case FMN_DIR_W: xform=FMN_XFORM_XREV; break;
  }
  TILE(-1,-8,tileid,xform)
}

/* Render.
 */
 
void fmn_hero_render(struct fmn_image *dst,int16_t scrollx,int16_t scrolly,struct fmn_sprite *sprite) {
  fmn_hero.framec++;
  int16_t dstx=((sprite->x-scrollx)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  int16_t dsty=((sprite->y-scrolly)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  
  // Some states do their own thing entirely.
  switch (fmn_hero.action_in_progress) {
    case FMN_ACTION_BROOM: fmn_hero_render_broom(dst,dstx,dsty); return;
    case FMN_ACTION_WAND: fmn_hero_render_wand(dst,dstx,dsty); return;
  }
  
  // Facing north, draw the arms first, and item on the right.
  // We don't care about the actions in this case, let them operate defaultly.
  if (fmn_hero.facedir==FMN_DIR_N) {
    fmn_hero_render_arm(dst,dstx,dsty,-1,0);
    fmn_hero_render_arm(dst,dstx,dsty,1,1);
    fmn_hero_render_body(dst,dstx,dsty);
    fmn_hero_render_head(dst,dstx,dsty);
    
  // When feather or umbrella is in use, it goes in the forward hand.
  } else if (
    (fmn_hero.action_in_progress==FMN_ACTION_FEATHER)||
    (fmn_hero.action_in_progress==FMN_ACTION_UMBRELLA)
  ) {
    switch (fmn_hero.facedir) {
      case FMN_DIR_W: fmn_hero_render_arm(dst,dstx,dsty,1,0); break;
      case FMN_DIR_E: fmn_hero_render_arm(dst,dstx,dsty,-1,0); break;
      case FMN_DIR_S: fmn_hero_render_arm(dst,dstx,dsty,1,0); break;
    }
    fmn_hero_render_body(dst,dstx,dsty);
    if (fmn_hero.facedir==FMN_DIR_S) {
      // One more exception: Facing south, draw the action arm before the head, to occlude the umbrella hand in motion.
      fmn_hero_render_arm(dst,dstx,dsty,1,1);
      fmn_hero_render_head(dst,dstx,dsty);
    } else {
      fmn_hero_render_head(dst,dstx,dsty);
      fmn_hero_render_arm(dst,dstx,dsty,(fmn_hero.facedir==FMN_DIR_W)?-1:1,1);
    }
    
  // East, west, and south are pretty similar.
  // Item goes in the foreground hand, which is opposite the direction of travel.
  } else {
    fmn_hero_render_arm(dst,dstx,dsty,(fmn_hero.facedir==FMN_DIR_W)?-1:1,0);
    fmn_hero_render_body(dst,dstx,dsty);
    fmn_hero_render_head(dst,dstx,dsty);
    fmn_hero_render_arm(dst,dstx,dsty,(fmn_hero.facedir==FMN_DIR_W)?1:-1,1);
  }
}
