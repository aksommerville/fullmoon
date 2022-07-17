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