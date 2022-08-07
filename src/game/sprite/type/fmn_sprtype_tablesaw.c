#include "game/fullmoon.h"
#include "game/model/fmn_map.h"
#include "game/sprite/hero/fmn_hero.h"
#include "game/sprite/fmn_sprite.h"

#define colc sprite->bv[0]
#define phase sprite->bv[1]
#define col sprite->bv[3]
#define dx sprite->sv[0]
#define trackx sprite->sv[1]
#define trackw sprite->sv[2]
#define ext sprite->sv[3]
#define extd sprite->sv[4]

#define FMN_TABLESAW_SPEED 7
#define FMN_TABLESAW_EXT_MIN ((FMN_MM_PER_TILE*3)/8)
#define FMN_TABLESAW_EXT_MAX ((FMN_MM_PER_TILE*15)/8)
#define FMN_TABLESAW_EXT_SPEED 9
#define FMN_TABLESAW_BLADE_W_MM ((FMN_MM_PER_TILE*4)/8)
#define FMN_TABLESAW_DANGER_UP_MM ((FMN_MM_PER_TILE*4)/8)
#define FMN_TABLESAW_DANGER_DOWN_MM ((FMN_MM_PER_TILE*2)/8)

/* Setup.
 */
 
static int8_t _tablesaw_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  if (colc<2) colc=2;
  col=sprite->x/FMN_MM_PER_TILE;
  trackx=col*FMN_MM_PER_TILE;
  trackw=(colc-1)*FMN_MM_PER_TILE; // 3+3 for margins, 8 for the blade's width
  dx=1;
  sprite->x=trackx; // x is the left edge of the blade
  sprite->x+=(phase*trackw)>>8;
  ext=FMN_TABLESAW_EXT_MAX;
  extd=-1;
  return 0;
}

/* Update.
 */
 
static void _tablesaw_update(struct fmn_sprite *sprite) {

  // Position.
  sprite->x+=dx*FMN_TABLESAW_SPEED;
  if (
    ((sprite->x<trackx)&&(dx<0))||
    ((sprite->x>trackx+trackw)&&(dx>0))
  ) dx=-dx;
  
  // Extension.
  ext+=extd*FMN_TABLESAW_EXT_SPEED;
  if (
    ((ext<FMN_TABLESAW_EXT_MIN)&&(extd<0))||
    ((ext>FMN_TABLESAW_EXT_MAX)&&(extd>0))
  ) extd=-extd;

  // Check hero.
  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  if (
    (herox>=sprite->x)&&
    (herox<sprite->x+FMN_TABLESAW_BLADE_W_MM)&&
    (heroy>=sprite->y-FMN_TABLESAW_DANGER_UP_MM)&&
    (heroy<sprite->y+FMN_TABLESAW_DANGER_DOWN_MM)
  ) {
    fmn_hero_injure(sprite);
  }
}

/* Render.
 */
 
static void _tablesaw_render(
  struct fmn_image *dst,
  int16_t scrollx,int16_t scrolly,
  struct fmn_sprite *sprite
) {

  // Track is tiles +0 for the edge and +16 middle. Images are oriented vertically, to pack them nicer.
  int16_t dstx=((col*FMN_MM_PER_TILE-scrollx)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  int16_t dsty=((sprite->y-scrolly-(FMN_MM_PER_TILE>>1))*FMN_TILESIZE)/FMN_MM_PER_TILE;
  fmn_blit_tile(dst,dstx,dsty,sprite->image,sprite->tileid,FMN_XFORM_SWAP|FMN_XFORM_XREV);
  dstx+=FMN_TILESIZE;
  uint8_t i=colc-2;
  for (;i-->0;dstx+=FMN_TILESIZE) fmn_blit_tile(dst,dstx,dsty,sprite->image,sprite->tileid+0x10,FMN_XFORM_SWAP);
  fmn_blit_tile(dst,dstx,dsty,sprite->image,sprite->tileid,FMN_XFORM_SWAP|FMN_XFORM_YREV|FMN_XFORM_XREV);

  // Blade is two tiles, just right of the track tiles.
  int16_t bladeh=(ext*FMN_TILESIZE)/FMN_MM_PER_TILE;
  dstx=((sprite->x-scrollx)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  dsty=dsty+4*FMN_GFXSCALE-bladeh;
  int16_t srcx=((sprite->tileid+1)&0x0f)*FMN_TILESIZE;
  int16_t srcy=(sprite->tileid>>4)*FMN_TILESIZE;
  fmn_blit(dst,dstx,dsty,sprite->image,srcx,srcy,FMN_TILESIZE,bladeh,(dx>0)?FMN_XFORM_XREV:0);
}

/* Type definition.
 */
 
const struct fmn_sprtype fmn_sprtype_tablesaw={
  .name="tablesaw",
  .init=_tablesaw_init,
  .update=_tablesaw_update,
  .render=_tablesaw_render,
};
