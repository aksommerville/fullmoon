#include "game/fullmoon.h"
#include "game/fmn_data.h"
#include "fmn_map.h"

/* Globals.
 */
 
static const struct fmn_map *fmn_map=0;
static int16_t fmn_vx=0;
static int16_t fmn_vy=0;
static uint8_t fmn_panicmap[FMN_SCREENW_TILES*FMN_SCREENH_TILES]={0};

/* Reset.
 */
 
void fmn_map_reset() {
  fmn_map=0;
  fmn_vx=fmn_vy=0;
  fmn_map_load_default(&outermap);
}

/* Get current view.
 */
 
uint8_t *fmn_map_get_view(uint8_t *stride) {
  if (fmn_map) {
    *stride=fmn_map->w;
    return fmn_map->v+fmn_vy*fmn_map->w+fmn_vx;
  }
  *stride=FMN_SCREENW_TILES;
  return fmn_panicmap;
}

/* Shift view.
 */
 
uint8_t fmn_map_navigate(int8_t dx,int8_t dy) {
  if (!fmn_map) return 0;
  
  int16_t nvx=fmn_vx+dx*FMN_SCREENW_TILES;
  if (nvx<0) return 0;
  if (nvx>fmn_map->w-FMN_SCREENW_TILES) return 0;
  int16_t nvy=fmn_vy+dy*FMN_SCREENH_TILES;
  if (nvy<0) return 0;
  if (nvy>fmn_map->h-FMN_SCREENH_TILES) return 0;
  if ((nvx==fmn_vx)&&(nvy==fmn_vy)) return 0;
  
  fmn_vx=nvx;
  fmn_vy=nvy;
  return 1;
}

/* Load new map.
 */
 
uint8_t fmn_map_load_default(const struct fmn_map *map) {
  if (!map) return 0;
  return fmn_map_load_position(map,map->initx,map->inity);
}

uint8_t fmn_map_load_position(const struct fmn_map *map,uint8_t x,uint8_t y) {
  if (!map) return 0;
  int16_t vx=(x/FMN_SCREENW_TILES)*FMN_SCREENW_TILES;
  int16_t vy=(y/FMN_SCREENH_TILES)*FMN_SCREENH_TILES;
  if (vx>map->w-FMN_SCREENW_TILES) return 0;
  if (vy>map->h-FMN_SCREENH_TILES) return 0;
  fmn_map=map;
  fmn_vx=vx;
  fmn_vy=vy;
  return 1;
}

/* Trivial accessors.
 */
 
void fmn_map_get_init_position(uint8_t *x,uint8_t *y) {
  if (fmn_map) {
    *x=fmn_map->initx;
    *y=fmn_map->inity;
  } else {
    *x=FMN_SCREENW_TILES>>1;
    *y=FMN_SCREENH_TILES>>1;
  }
}

void fmn_map_get_scroll(uint8_t *x,uint8_t *y) {
  *x=fmn_vx;
  *y=fmn_vy;
}

void fmn_map_get_size(uint8_t *w,uint8_t *h) {
  if (fmn_map) {
    *w=fmn_map->w;
    *h=fmn_map->h;
  } else {
    *w=FMN_SCREENW_TILES;
    *h=FMN_SCREENH_TILES;
  }
}

void fmn_map_get_size_mm(int16_t *wmm,int16_t *hmm) {
  if (fmn_map) {
    *wmm=fmn_map->w*FMN_MM_PER_TILE;
    *hmm=fmn_map->h*FMN_MM_PER_TILE;
  } else {
    *wmm=FMN_SCREENW_MM;
    *hmm=FMN_SCREENH_MM;
  }
}

/* Tile properties.
 */
 
static uint8_t fmn_map_tile_is_solid(uint8_t tile) {
  //TODO Need to store this stuff somewhere. For now, the top two rows are passable, bottom 14 solid.
  if (tile<0x20) return 0;
  return 1;
}

/* Check collisions.
 */
 
static uint8_t fmn_map_check_collision_1(int16_t *adjx,int16_t *adjy,int16_t x,int16_t y,int16_t w,int16_t h,uint8_t panic) {
  if (!panic--) { if (adjx) *adjx=0; if (adjy) *adjy=0; return 1; }
  if ((w<1)||(h<1)) return 0;
  if (!fmn_map) return 0; // the fallback map has no solid cells
  
  int16_t cola=x/FMN_MM_PER_TILE; if (x<0) cola--;
  int16_t rowa=y/FMN_MM_PER_TILE; if (y<0) rowa--;
  int16_t colz=(x+w-1)/FMN_MM_PER_TILE; if (x+w-1<0) colz--;
  int16_t rowz=(y+h-1)/FMN_MM_PER_TILE; if (y+h-1<0) rowz--;
  if (cola<0) cola=0;
  if (rowa<0) rowa=0;
  if (colz>=fmn_map->w) colz=fmn_map->w-1;
  if (rowz>=fmn_map->h) rowz=fmn_map->h-1;
  if (cola>colz) return 0;
  if (rowa>rowz) return 0;
  
  const uint8_t *srcrow=fmn_map->v+rowa*fmn_map->w+cola;
  int16_t row=rowa;
  for (;row<=rowz;row++,srcrow+=fmn_map->w) {
    const uint8_t *srcp=srcrow;
    int16_t col=cola;
    for (;col<=colz;col++,srcp++) {
      if (!fmn_map_tile_is_solid(*srcp)) continue;
      if (!adjx||!adjy) return 1;
      
      int16_t hardl=col*FMN_MM_PER_TILE;
      int16_t hardt=row*FMN_MM_PER_TILE;
      int16_t hardr=hardl+FMN_MM_PER_TILE;
      int16_t hardb=hardt+FMN_MM_PER_TILE;
        
      int16_t escl=x+w-hardl;
      int16_t escr=hardr-x;
      int16_t esct=y+h-hardt;
      int16_t escb=hardb-y;
      int16_t escx=(escl<=escr)?-escl:escr;
      int16_t escy=(esct<=escb)?-esct:escb;
      
      // First cell collision, and we have two candidate resolutions: escx and escy.
      // Try each of those. If they are legal or correct on the other axis, great. Otherwise report unresolvable.
      int16_t xadjx=0,xadjy=0,yadjx=0,yadjy=0;
      uint8_t xcoll=fmn_map_check_collision_1(&xadjx,&xadjy,x+escx,y,w,h,panic);
      uint8_t ycoll=fmn_map_check_collision_1(&yadjx,&yadjy,x,y+escy,w,h,panic);
      // Both collided? Unresolvable.
      if (xcoll&&ycoll) { *adjx=*adjy=0; return 1; }
      // One collided? Use the other.
      if (xcoll&&!ycoll) { *adjx=0; *adjy=escy; return 1; }
      if (!xcoll&&ycoll) { *adjx=escx; *adjy=0; return 1; }
      // Both valid. Use whichever has the shortest escapement.
      int16_t ax=(escx<0)?-escx:escx;
      int16_t ay=(escy<0)?-escy:escy;
      if (ax<=ay) { *adjx=escx; *adjy=0; return 1; }
      *adjx=0;
      *adjy=escy;
      return 1;
    }
  }
  
  return 0;
}

uint8_t fmn_map_check_collision(int16_t *adjx,int16_t *adjy,int16_t x,int16_t y,int16_t w,int16_t h) {
  return fmn_map_check_collision_1(adjx,adjy,x,y,w,h,3);
}
