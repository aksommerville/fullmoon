#include "game/fullmoon.h"
#include "game/model/fmn_map.h"
#include "game/sprite/fmn_sprite.h"
#include "game/sprite/hero/fmn_hero.h"

#define frontdir sprite->bv[0]
#define animframe sprite->bv[3]
#define animclock sprite->bv[4]
#define triggered sprite->bv[5]
#define expandrange sprite->sv[0]
#define expansion sprite->sv[1]

#define ANIMTIME 8
#define FMN_FIREWALL_RANGE_LIMIT FMN_SCREENW_TILES
#define FMN_FIREWALL_ADVANCE_SPEED 4 /* TODO */
#define FMN_FIREWALL_RETREAT_SPEED 1

/* Setup.
 */
 
static uint8_t fmn_firewall_detect_solid(
  const struct fmn_map *map,
  uint8_t x,uint8_t y,uint8_t w,uint8_t h
) {
  const uint8_t *row=map->v+y*map->w+x;
  for (;h-->0;row+=map->w) {
    const uint8_t *p=row;
    uint8_t xi=w;
    for (;xi-->0;p++) {
      if (map->tileprops[*p]&FMN_TILE_SOLID) return 1;
    }
  }
  return 0;
}

static void fmn_firewall_fill(struct fmn_sprite *sprite) {
  int16_t col=sprite->x/FMN_MM_PER_TILE;
  int16_t row=sprite->y/FMN_MM_PER_TILE;
  int16_t colc=1,rowc=1;
  const struct fmn_map *map=fmn_map_get();
  if (!map||!map->tileprops||(col<0)||(row<0)||(col>=map->w)||(row>=map->h)) {
    fmn_sprite_del_later(sprite);
    return;
  }
  switch (frontdir) {
  
    // Expand horizontally.
    case FMN_DIR_N: case FMN_DIR_S: {
        while ((col>0)&&!(map->tileprops[map->v[row*map->w+col-1]]&FMN_TILE_SOLID)) { col--; colc++; }
        while ((col+colc<map->w)&&!(map->tileprops[map->v[row*map->w+col+colc]]&FMN_TILE_SOLID)) colc++;
      } break;
      
    // Expand vertically.
    case FMN_DIR_E: case FMN_DIR_W: {
        while ((row>0)&&!(map->tileprops[map->v[(row-1)*map->w+col]]&FMN_TILE_SOLID)) { row--; rowc++; }
        while ((row+rowc<map->h)&&!(map->tileprops[map->v[(row+rowc)*map->w+col]]&FMN_TILE_SOLID)) rowc++;
      } break;
      
    default: { // invalid!
        fmn_sprite_del_later(sprite);
      } return;
  }
  
  // Commit resting bounds.
  sprite->x=col*FMN_MM_PER_TILE;
  sprite->y=row*FMN_MM_PER_TILE;
  sprite->w=colc*FMN_MM_PER_TILE;
  sprite->h=rowc*FMN_MM_PER_TILE;
  
  // Measure expandrange, to the map edge, the first solid, or FMN_FIREWALL_RANGE_LIMIT. First measure in cells.
  switch (frontdir) {
    case FMN_DIR_N: {
        while (1) {
          if (expandrange>=FMN_FIREWALL_RANGE_LIMIT) break;
          if (expandrange>=row) break;
          if (fmn_firewall_detect_solid(map,col,row-expandrange-1,colc,1)) break;
          expandrange++;
        }
      } break;
    case FMN_DIR_S: {
        while (1) {
          if (expandrange>=FMN_FIREWALL_RANGE_LIMIT) break;
          if (row+1+expandrange>=map->h) break;
          if (fmn_firewall_detect_solid(map,col,row+1+expandrange,colc,1)) break;
          expandrange++;
        }
      } break;
    case FMN_DIR_W: {
        while (1) {
          if (expandrange>=FMN_FIREWALL_RANGE_LIMIT) break;
          if (expandrange>=col) break;
          if (fmn_firewall_detect_solid(map,col-expandrange-1,row,1,rowc)) break;
          expandrange++;
        }
      } break;
    case FMN_DIR_E: {
        while (1) {
          if (expandrange>=FMN_FIREWALL_RANGE_LIMIT) break;
          if (col+1+expandrange>=map->w) break;
          if (fmn_firewall_detect_solid(map,col+1+expandrange,row,1,rowc)) break;
          expandrange++;
        }
      } break;
  }
  expandrange*=FMN_MM_PER_TILE;
}

static int8_t _fmn_firewall_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  fmn_firewall_fill(sprite);
  return 0;
}

/* Update.
 */
 
static uint8_t fmn_firewall_begazed(const struct fmn_sprite *sprite) {
  uint8_t herodir=fmn_hero_get_facedir();
  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  switch (frontdir) {
    case FMN_DIR_S: {
        if (herodir!=FMN_DIR_N) return 0;
        if (herox<sprite->x) return 0;
        if (herox>=sprite->x+sprite->w) return 0;
        return 1;
      }
    case FMN_DIR_N: {
        if (herodir!=FMN_DIR_S) return 0;
        if (herox<sprite->x) return 0;
        if (herox>=sprite->x+sprite->w) return 0;
        return 1;
      }
    case FMN_DIR_E: {
        if (herodir!=FMN_DIR_W) return 0;
        if (heroy<sprite->y) return 0;
        if (heroy>=sprite->y+sprite->h) return 0;
        return 1;
      }
    case FMN_DIR_W: {
        if (herodir!=FMN_DIR_E) return 0;
        if (heroy<sprite->y) return 0;
        if (heroy>=sprite->y+sprite->h) return 0;
        return 1;
      }
  }
  return 0;
}

static void _fmn_firewall_update(struct fmn_sprite *sprite) {

  // Animate always.
  animclock++;
  if (animclock>=ANIMTIME) {
    animclock=0;
    animframe^=1;
  }
  
  // Advance or retreat.
  if (triggered) {
    if (expansion<expandrange) {
      expansion+=FMN_FIREWALL_ADVANCE_SPEED;
      if (expansion>expandrange) expansion=expandrange;
    } else {
      if (!fmn_firewall_begazed(sprite)) {
        triggered=0;
      }
    }
  } else {
    if (expansion>0) {
      expansion-=FMN_FIREWALL_RETREAT_SPEED;
      if (expansion<0) expansion=0;
    }
    if (fmn_firewall_begazed(sprite)) {
      triggered=1;
    }
  }
  switch (frontdir) {
    case FMN_DIR_S: sprite->h=FMN_MM_PER_TILE+expansion; break;
    case FMN_DIR_N: { int16_t bottom=sprite->y+sprite->h; sprite->h=FMN_MM_PER_TILE+expansion; sprite->y=bottom-sprite->h; } break;
    case FMN_DIR_E: sprite->w=FMN_MM_PER_TILE+expansion; break;
    case FMN_DIR_W: { int16_t right=sprite->x+sprite->w; sprite->w=FMN_MM_PER_TILE+expansion; sprite->x=right-sprite->w; } break;
  }
  
  // Hurt hero if overlap.
  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  if (
    (herox>=sprite->x)&&
    (heroy>=sprite->y)&&
    (herox<sprite->x+sprite->w)&&
    (heroy<sprite->y+sprite->h)
  ) {
    fmn_hero_injure(sprite);
  }
}

/* Render.
 */
 
static void fmn_firewall_render_internal(
  struct fmn_image *dst,
  int16_t dstx,int16_t dsty,
  uint8_t minorc,int8_t minordx,int8_t minordy, // wide axis
  uint8_t majorc,int8_t majordx,int8_t majordy, // stretchy axis
  const struct fmn_image *src,
  uint8_t tileid,uint8_t xform
) {
  uint8_t addtileid=0; // first row
  for (;majorc-->0;dstx+=majordx*FMN_TILESIZE,dsty+=majordy*FMN_TILESIZE) {
    fmn_blit_tile(dst,dstx,dsty,src,tileid+addtileid,xform);
    if (minorc>=2) {
      fmn_blit_tile(dst,
        minordx?(dstx+(minorc-1)*minordx*FMN_TILESIZE):dstx,
        minordy?(dsty+(minorc-1)*minordy*FMN_TILESIZE):dsty,
        src,tileid+addtileid,xform|FMN_XFORM_XREV
      );
      uint8_t i=minorc-2;
      int16_t x=dstx+minordx*FMN_TILESIZE;
      int16_t y=dsty+minordy*FMN_TILESIZE;
      for (;i-->0;x+=minordx*FMN_TILESIZE,y+=minordy*FMN_TILESIZE) {
        fmn_blit_tile(dst,x,y,src,tileid+addtileid+0x01,xform);
      }
    }
    addtileid=0x10; // for all rows after the first
  }
}

static void _fmn_firewall_render(
  struct fmn_image *dst,
  int16_t scrollx,int16_t scrolly,
  struct fmn_sprite *sprite
) {
  int16_t dstx=((sprite->x-scrollx)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  int16_t dsty=((sprite->y-scrolly)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  
  // The wide axis will always land on tile boundaries.
  // The stretchy axis is one tile at idle, but can grow. We will round up, nice and simple.
  // firewalls must be placed such that we can overflow the stretchy axis offscreen.
  uint8_t colc=(sprite->w+FMN_MM_PER_TILE-1)/FMN_MM_PER_TILE;
  uint8_t rowc=(sprite->h+FMN_MM_PER_TILE-1)/FMN_MM_PER_TILE;
  
  uint8_t tileid=sprite->tileid+(animframe<<1); // corner; +0x01=front, +0x10=edge, +0x11=fill; default orientation NW corner
  switch (frontdir) {
    case FMN_DIR_N: fmn_firewall_render_internal(dst,
        dstx,
        dsty,
        colc,1,0,
        rowc,0,1,
        sprite->image,tileid,0
      ); break;
    case FMN_DIR_S: fmn_firewall_render_internal(dst,
        dstx,
        dsty+(sprite->h*FMN_TILESIZE)/FMN_MM_PER_TILE-FMN_TILESIZE,
        colc,1,0,
        rowc,0,-1,
        sprite->image,tileid,FMN_XFORM_YREV
      ); break;
    case FMN_DIR_E: fmn_firewall_render_internal(dst,
        dstx+(sprite->w*FMN_TILESIZE)/FMN_MM_PER_TILE-FMN_TILESIZE,
        dsty,
        rowc,0,1,
        colc,-1,0,
        sprite->image,tileid,FMN_XFORM_YREV|FMN_XFORM_SWAP
      ); break;
    case FMN_DIR_W: fmn_firewall_render_internal(dst,
        dstx,
        dsty,
        rowc,0,1,
        colc,1,0,
        sprite->image,tileid,FMN_XFORM_SWAP
      ); break;
  }
}

/* Type definition.
 */

const struct fmn_sprtype fmn_sprtype_firewall={
  .name="firewall",
  .init=_fmn_firewall_init,
  .update=_fmn_firewall_update,
  .render=_fmn_firewall_render,
};
