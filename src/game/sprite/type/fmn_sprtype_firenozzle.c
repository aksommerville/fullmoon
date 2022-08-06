#include "game/fullmoon.h"
#include "game/ui/fmn_play.h"
#include "game/model/fmn_map.h"
#include "game/sprite/hero/fmn_hero.h"
#include "game/sprite/fmn_sprite.h"
#include <stdio.h>

// If the three POI parameters are zero, the nozzle is always on.
#define gstatep sprite->bv[0] /* 0 means "no switch", always on */
#define proximity_enable sprite->bv[1] /* 0,1 */
#define period sprite->bv[2] /* period in frames, or zero for always on. 120..200 seems good */

#define stage sprite->bv[4]
#define clock sprite->bv[5]
#define lockon sprite->bv[6] /* for the lucky nozzle that burns a witch -- it gets to stay on forever */

// (sv) are two rectangles: My proximity zone and my kill zone.
// These are constant after init, the kill zone stays put even when we are turned off.
#define proxx sprite->sv[0]
#define proxy sprite->sv[1]
#define proxw sprite->sv[2]
#define proxh sprite->sv[3]
#define killx sprite->sv[4]
#define killy sprite->sv[5]
#define killw sprite->sv[6]
#define killh sprite->sv[7]

// "LENGTH" is the longer axis, "BREADTH" the shorter. PROXIMITY_LENGTH includes the nozzle itself, KILL does not.
#define FMN_FIRENOZZLE_PROXIMITY_LENGTH (FMN_MM_PER_TILE*5)
#define FMN_FIRENOZZLE_PROXIMITY_BREADTH ((FMN_MM_PER_TILE*4)/2)
#define FMN_FIRENOZZLE_KILL_LENGTH (FMN_MM_PER_TILE*3)
#define FMN_FIRENOZZLE_KILL_BREADTH (FMN_MM_PER_TILE*1)

#define FMN_FIRENOZZLE_STAGE_OFF 0
#define FMN_FIRENOZZLE_STAGE_WARMUP 1
#define FMN_FIRENOZZLE_STAGE_BURN 2
#define FMN_FIRENOZZLE_STAGE_COOLOFF 3

#define FMN_FIRENOZZLE_WARMUP_TIME 15

/* Setup.
 */
 
static int8_t _firenozzle_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {

  sprite->w=FMN_MM_PER_TILE;
  sprite->h=FMN_MM_PER_TILE;
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  
  // Embed into the wall a little bit, according to our xform.
  // The natural orientation is facing east.
  int16_t embeddepth=(FMN_MM_PER_TILE*3)/8;
  switch (sprite->xform&(FMN_XFORM_XREV|FMN_XFORM_SWAP)) {
    case 0: { // E
        sprite->x-=embeddepth;
        proxx=sprite->x;
        proxy=sprite->y+(sprite->h>>1)-(FMN_FIRENOZZLE_PROXIMITY_BREADTH>>1);
        proxw=FMN_FIRENOZZLE_PROXIMITY_LENGTH;
        proxh=FMN_FIRENOZZLE_PROXIMITY_BREADTH;
        killx=sprite->x+sprite->w-(FMN_MM_PER_TILE>>1);
        killy=sprite->y+(sprite->h>>1)-(FMN_FIRENOZZLE_KILL_BREADTH>>1);
        killw=FMN_FIRENOZZLE_KILL_LENGTH+(FMN_MM_PER_TILE>>1);
        killh=FMN_FIRENOZZLE_KILL_BREADTH;
      } break;
    case FMN_XFORM_XREV: { // W
        sprite->x+=embeddepth;
        proxx=sprite->x+sprite->w-FMN_FIRENOZZLE_PROXIMITY_LENGTH;
        proxy=sprite->y+(sprite->h>>1)-(FMN_FIRENOZZLE_PROXIMITY_BREADTH>>1);
        proxw=FMN_FIRENOZZLE_PROXIMITY_LENGTH;
        proxh=FMN_FIRENOZZLE_PROXIMITY_BREADTH;
        killx=sprite->x-FMN_FIRENOZZLE_KILL_LENGTH;
        killy=sprite->y+(sprite->h>>1)-(FMN_FIRENOZZLE_KILL_BREADTH>>1);
        killw=FMN_FIRENOZZLE_KILL_LENGTH+(FMN_MM_PER_TILE>>1);
        killh=FMN_FIRENOZZLE_KILL_BREADTH;
      } break;
    case FMN_XFORM_SWAP: { // S
        sprite->y-=embeddepth;
        proxx=sprite->x+(sprite->w>>1)-(FMN_FIRENOZZLE_PROXIMITY_BREADTH>>1);
        proxy=sprite->y;
        proxw=FMN_FIRENOZZLE_PROXIMITY_BREADTH;
        proxh=FMN_FIRENOZZLE_PROXIMITY_LENGTH;
        killx=sprite->x+(sprite->w>>1)-(FMN_FIRENOZZLE_KILL_BREADTH>>1);
        killy=sprite->y+sprite->h-(FMN_MM_PER_TILE>>1);
        killw=FMN_FIRENOZZLE_KILL_BREADTH;
        killh=FMN_FIRENOZZLE_KILL_LENGTH+(FMN_MM_PER_TILE>>1);
      } break;
    case FMN_XFORM_SWAP|FMN_XFORM_XREV: { // N
        sprite->y+=embeddepth;
        proxx=sprite->x+(sprite->w>>1)-(FMN_FIRENOZZLE_PROXIMITY_BREADTH>>1);
        proxy=sprite->y+sprite->h-FMN_FIRENOZZLE_PROXIMITY_LENGTH;
        proxw=FMN_FIRENOZZLE_PROXIMITY_BREADTH;
        proxh=FMN_FIRENOZZLE_PROXIMITY_LENGTH;
        killx=sprite->x+(sprite->w>>1)-(FMN_FIRENOZZLE_KILL_BREADTH>>1);
        killy=sprite->y-FMN_FIRENOZZLE_KILL_LENGTH;
        killw=FMN_FIRENOZZLE_KILL_BREADTH;
        killh=FMN_FIRENOZZLE_KILL_LENGTH+(FMN_MM_PER_TILE>>1);
      } break;
  }

  return 0;
}

/* Update.
 */
 
static void _firenozzle_update(struct fmn_sprite *sprite) {

  // lockon overrides everything else.
  if (lockon) {
    stage=FMN_FIRENOZZLE_STAGE_BURN;
    return;
  }

  // Check gstate first if in play -- a false here tells us everything we need to know.
  if (gstatep) {
    if (!fmn_gstate[gstatep]) {
      stage=FMN_FIRENOZZLE_STAGE_OFF;
      return;
    }
  }

  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  
  // Proximity test.
  uint8_t proxok=0;
  if (!proximity_enable) proxok=1;
  else if ((herox>=proxx)&&(heroy>=proxy)&&(herox<proxx+proxw)&&(heroy<proxy+proxh)) proxok=1;
  if (!proxok) {
    stage=FMN_FIRENOZZLE_STAGE_OFF;
    clock=0;
    return;
  }
  
  // If the clock is in play, advance it and consult it for our stage.
  // Otherwise, we are burning.
  if (period) {
    clock++;
    if (clock>=period) clock=0;
    if (clock<FMN_FIRENOZZLE_WARMUP_TIME) stage=FMN_FIRENOZZLE_STAGE_WARMUP;
    else if (clock<(period>>1)) stage=FMN_FIRENOZZLE_STAGE_BURN;
    else stage=FMN_FIRENOZZLE_STAGE_OFF;
  } else {
    stage=FMN_FIRENOZZLE_STAGE_BURN;
  }
  
  // Burn the witch.
  if (stage==FMN_FIRENOZZLE_STAGE_BURN) {
    if ((herox>=killx)&&(heroy>=killy)&&(herox<killx+killw)&&(heroy<killy+killh)) {
      fmn_hero_injure(sprite);
      lockon=1;
    }
  }
}

/* Render.
 */
 
static void _firenozzle_render(
  struct fmn_image *dst,
  int16_t scrollx,int16_t scrolly,
  struct fmn_sprite *sprite
) {
  if (!sprite||!sprite->image) return;
  int16_t dstx=((sprite->x+(sprite->w>>1)-scrollx)*FMN_TILESIZE)/FMN_MM_PER_TILE-(FMN_TILESIZE>>1);
  int16_t dsty=((sprite->y+(sprite->h>>1)-scrolly)*FMN_TILESIZE)/FMN_MM_PER_TILE-(FMN_TILESIZE>>1);
  
  uint8_t animframe;
  uint8_t tileid=sprite->tileid;
  switch (stage) {
    case FMN_FIRENOZZLE_STAGE_WARMUP: tileid+=1; break;
    case FMN_FIRENOZZLE_STAGE_BURN: animframe=((fmn_play_frame_count/10)%3); tileid+=2+animframe; break;
  }
  fmn_blit_tile(dst,dstx,dsty,sprite->image,tileid,sprite->xform);
  
  // If burning, draw two tiles of shaft and one of nub.
  // These both animate in sync with the base.
  if (stage==FMN_FIRENOZZLE_STAGE_BURN) {
    int16_t dx,dy;
    switch (sprite->xform&(FMN_XFORM_SWAP|FMN_XFORM_XREV)) {
      case 0: dx=FMN_TILESIZE; dy=0; break;
      case FMN_XFORM_XREV: dx=-FMN_TILESIZE; dy=0; break;
      case FMN_XFORM_SWAP: dx=0; dy=FMN_TILESIZE; break;
      default: dx=0; dy=-FMN_TILESIZE; break;
    }
    fmn_blit_tile(dst,dstx+dx*1,dsty+dy*1,sprite->image,sprite->tileid+5+animframe,sprite->xform);
    fmn_blit_tile(dst,dstx+dx*2,dsty+dy*2,sprite->image,sprite->tileid+5+animframe,sprite->xform);
    fmn_blit_tile(dst,dstx+dx*3,dsty+dy*3,sprite->image,sprite->tileid+8+animframe,sprite->xform);
  }
}

/* Type definition.
 */
 
const struct fmn_sprtype fmn_sprtype_firenozzle={
  .name="firenozzle",
  .init=_firenozzle_init,
  .update=_firenozzle_update,
  .render=_firenozzle_render,
};
