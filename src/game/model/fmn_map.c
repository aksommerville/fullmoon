#include "game/fullmoon.h"
#include "game/fmn_data.h"
#include "game/fmn_play.h"
#include "game/model/fmn_hero.h"
#include "game/model/fmn_proximity.h"
#include "game/sprite/fmn_sprite.h"
#include "game/model/fmn_map.h"

/* Globals.
 */
 
static const struct fmn_map *fmn_map=0;
static int16_t fmn_vx=0;
static int16_t fmn_vy=0;
static uint8_t fmn_panicmap[FMN_SCREENW_TILES*FMN_SCREENH_TILES]={0};

/* Region heads.
 */
 
const struct fmn_map *map_region_heads[8]={
  &cheatertrap, // reserved
  &home,        // "home"
  &cheatertrap, // "forest" TODO
  &cheatertrap, // "caves" TODO
  &cheatertrap, // "desert" TODO
  &cheatertrap, // "swamp" TODO
  &cheatertrap, // "castle" TODO
  &cheatertrap, // cheatertrap, for real
};

/* Reset.
 */
 
void fmn_map_reset() {
  fmn_map=0;
  fmn_vx=fmn_vy=0;
  fmn_proximity_clear();
  fmn_map_load_default(&home);
}

void fmn_map_reset_region(uint8_t region) {
  fmn_map=0;
  fmn_vx=fmn_vy=0;
  fmn_proximity_clear();
  fmn_map_load_default(map_region_heads[region&7]);
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
  int16_t nvy=fmn_vy+dy*FMN_SCREENH_TILES;
  if ((nvx==fmn_vx)&&(nvy==fmn_vy)) return 0;
  int16_t hdx=0,hdy=0;
  const struct fmn_map *nextmap=fmn_map;
       if (nvx<0) { if (nextmap=fmn_map_find_edge_door_left(fmn_map,nvy,&hdx,&hdy)) { nvx=nextmap->w-FMN_SCREENW_TILES; nvy=fmn_vy+(hdy/FMN_MM_PER_TILE); }}
  else if (nvy<0) { if (nextmap=fmn_map_find_edge_door_up(fmn_map,nvx,&hdx,&hdy)) { nvx=fmn_vx+hdx/FMN_MM_PER_TILE; nvy=nextmap->h-FMN_SCREENH_TILES; }}
  else if (nvx>fmn_map->w-FMN_SCREENW_TILES) { if (nextmap=fmn_map_find_edge_door_right(fmn_map,nvy,&hdx,&hdy)) { nvx=0; nvy=fmn_vy+hdy/FMN_MM_PER_TILE; }}
  else if (nvy>fmn_map->h-FMN_SCREENH_TILES) { if (nextmap=fmn_map_find_edge_door_down(fmn_map,nvx,&hdx,&hdy)) { nvx=fmn_vx+hdx/FMN_MM_PER_TILE; nvy=0; }}
  if (!nextmap) return 0; // let her walk off the edge if there isn't a door
  uint8_t switched=(fmn_map!=nextmap);
  
  fmn_map_call_visibility_pois(0);
  fmn_proximity_clear();
  
  fmn_vx=nvx;
  fmn_vy=nvy;
  
  if (switched) {
    fmn_sprites_clear();
    fmn_map=nextmap;
    int16_t hx,hy;
    fmn_hero_get_world_position(&hx,&hy);
    fmn_hero_force_position(hx+hdx,hy+hdy);
    fmn_sprites_load();
  }
  
  fmn_map_add_proximity_pois();
  fmn_map_call_visibility_pois(1);
  
  return 1;
}

/* Load new map.
 */
 
uint8_t fmn_map_load_default(const struct fmn_map *map) {
  if (!map) return 0;
  uint8_t x=map->w>>1,y=map->h>>1;
  const struct fmn_map_poi *poi=map->poiv;
  uint16_t i=map->poic;
  for (;i-->0;poi++) {
    if (poi->q[0]!=FMN_POI_START) continue;
    x=poi->x;
    y=poi->y;
    break;
  }
  return fmn_map_load_position(map,x,y);
}

uint8_t fmn_map_load_position(const struct fmn_map *map,uint8_t x,uint8_t y) {
  if (!map) return 0;
  int16_t vx=(x/FMN_SCREENW_TILES)*FMN_SCREENW_TILES;
  int16_t vy=(y/FMN_SCREENH_TILES)*FMN_SCREENH_TILES;
  if (vx>map->w-FMN_SCREENW_TILES) return 0;
  if (vy>map->h-FMN_SCREENH_TILES) return 0;
  
  fmn_map_call_visibility_pois(0);
  fmn_proximity_clear();
  fmn_sprites_clear();
  
  fmn_map=map;
  fmn_vx=vx;
  fmn_vy=vy;

  fmn_hero_force_position(
    x*FMN_MM_PER_TILE,
    y*FMN_MM_PER_TILE
  );
  fmn_sprites_load();
  fmn_map_add_proximity_pois();
  fmn_map_call_visibility_pois(1);
  fmn_bgbits_dirty();
  
  return 1;
}

/* Trivial accessors.
 */
 
void fmn_map_get_init_position(uint8_t *x,uint8_t *y) {
  if (fmn_map) {
    const struct fmn_map_poi *poi=fmn_map->poiv;
    uint16_t i=fmn_map->poic;
    for (;i-->0;poi++) {
      if (poi->q[0]!=FMN_POI_START) continue;
      *x=poi->x;
      *y=poi->y;
      return;
    }
    *x=fmn_map->w>>1;
    *y=fmn_map->h>>1;
  } else {
    *x=FMN_SCREENW_TILES>>1;
    *y=FMN_SCREENH_TILES>>1;
  }
}

void fmn_map_get_scroll(uint8_t *x,uint8_t *y) {
  *x=fmn_vx;
  *y=fmn_vy;
}

void fmn_map_get_scroll_mm(int16_t *xmm,int16_t *ymm) {
  *xmm=fmn_vx*FMN_MM_PER_TILE;
  *ymm=fmn_vy*FMN_MM_PER_TILE;
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
 
static uint8_t fmn_map_tile_is_solid(uint8_t tile,uint8_t mask) {
  if (!fmn_map) return 0;
  if (!fmn_map->tileprops) return 0;
  return (fmn_map->tileprops[tile]&mask)?1:0;
}

/* Check collisions.
 */
 
static uint8_t fmn_map_check_collision_1(
  int16_t *adjx,int16_t *adjy,
  int16_t x,int16_t y,int16_t w,int16_t h,
  uint8_t panic,
  uint8_t collmask,uint16_t spriteflags
) {
  if (!panic--) { if (adjx) *adjx=0; if (adjy) *adjy=0; return 1; }
  if ((w<1)||(h<1)) return 0;
  if (!fmn_map) return 0; // the fallback map has no solid cells (and no spawn points, so there shouldn't be sprite collisions either)
  
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
  
  /* Check against the grid.
   * If this finds a collision, we will ignore sprite collisions.
   * Hopefully, an existing sprite collision will be re-detected later.
   */
  const uint8_t *srcrow=fmn_map->v+rowa*fmn_map->w+cola;
  int16_t row=rowa;
  for (;row<=rowz;row++,srcrow+=fmn_map->w) {
    const uint8_t *srcp=srcrow;
    int16_t col=cola;
    for (;col<=colz;col++,srcp++) {
      if (!fmn_map_tile_is_solid(*srcp,collmask)) continue;
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
      uint8_t xcoll=fmn_map_check_collision_1(&xadjx,&xadjy,x+escx,y,w,h,panic,collmask,spriteflags);
      uint8_t ycoll=fmn_map_check_collision_1(&yadjx,&yadjy,x,y+escy,w,h,panic,collmask,spriteflags);
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
  
  /* Check against sprites, if we're doing that.
   */
  if (spriteflags) {
    struct fmn_sprite **p=fmn_spritev;
    uint16_t i=fmn_spritec;
    for (;i-->0;p++) {
      struct fmn_sprite *sprite=*p;
      if (!(sprite->flags&spriteflags)) continue;
      
      // First rule out sprites based on their outer bounds, that's cheap.
      if (sprite->x>=x+w) continue;
      if (sprite->y>=y+h) continue;
      if (sprite->x+sprite->w<=x) continue;
      if (sprite->y+sprite->h<=y) continue;
      
      // Fetch its proper physical bounds for a closer look.
      int16_t px,py,pw,ph;
      sprite->type->hitbox(&px,&py,&pw,&ph,sprite);
      if ((pw<1)||(ph<1)) continue;
      px+=sprite->x;
      py+=sprite->y;
      if (px>=x+w) continue;
      if (py>=y+h) continue;
      if (px+pw<=x) continue;
      if (py+ph<=y) continue;
      
      // We have a collision. Determine escapement. It's the exact same thing we did above for grid cells.
      int16_t escl=x+w-px;
      int16_t escr=px+pw-x;
      int16_t esct=y+h-py;
      int16_t escb=py+ph-y;
      int16_t escx=(escl<=escr)?-escl:escr;
      int16_t escy=(esct<=escb)?-esct:escb;
      
      // First cell collision, and we have two candidate resolutions: escx and escy.
      // Try each of those. If they are legal or correct on the other axis, great. Otherwise report unresolvable.
      int16_t xadjx=0,xadjy=0,yadjx=0,yadjy=0;
      uint8_t xcoll=fmn_map_check_collision_1(&xadjx,&xadjy,x+escx,y,w,h,panic,collmask,spriteflags);
      uint8_t ycoll=fmn_map_check_collision_1(&yadjx,&yadjy,x,y+escy,w,h,panic,collmask,spriteflags);
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

uint8_t fmn_map_check_collision(
  int16_t *adjx,int16_t *adjy,
  int16_t x,int16_t y,int16_t w,int16_t h,
  uint8_t collmask,uint16_t spriteflags
) {
  if (!collmask) return 0;
  return fmn_map_check_collision_1(adjx,adjy,x,y,w,h,3,collmask,spriteflags);
}

/* Apply DOOR and TREADLE.
 */
 
uint8_t fmn_map_enter_cell(uint8_t x,uint8_t y) {
  if (!fmn_map) return 0;
  const struct fmn_map_poi *poi=fmn_map->poiv;
  uint16_t i=fmn_map->poic;
  for (;i-->0;poi++) {
    if (poi->x!=x) continue;
    if (poi->y!=y) continue;
    switch (poi->q[0]) {
      case FMN_POI_TREADLE: {
          void (*fn)(uint8_t,uint8_t,uint8_t,uint8_t)=poi->qp;
          fn(1,poi->q[1],poi->q[2],poi->q[3]);
        } break;
      case FMN_POI_DOOR: {
          return fmn_map_load_position(poi->qp,poi->q[1],poi->q[2]);
        } break;
    }
  }
  return 0;
}

/* Terminate TREADLE.
 */
 
void fmn_map_exit_cell(uint8_t x,uint8_t y) {
  if (!fmn_map) return;
  const struct fmn_map_poi *poi=fmn_map->poiv;
  uint16_t i=fmn_map->poic;
  for (;i-->0;poi++) {
    if (poi->x!=x) continue;
    if (poi->y!=y) continue;
    switch (poi->q[0]) {
      case FMN_POI_TREADLE: {
          void (*fn)(uint8_t,uint8_t,uint8_t,uint8_t)=poi->qp;
          fn(0,poi->q[1],poi->q[2],poi->q[3]);
        } break;
    }
  }
}

/* Call any VISIBILITY POI currently visible.
 */
 
void fmn_map_call_visibility_pois(uint8_t state) {
  if (!fmn_map) return;
  const struct fmn_map_poi *poi=fmn_map->poiv;
  uint16_t i=fmn_map->poic;
  for (;i-->0;poi++) {
    if (poi->q[0]!=FMN_POI_VISIBILITY) continue;
    if (poi->x<fmn_vx) continue;
    if (poi->y<fmn_vy) continue;
    if (poi->x>=fmn_vx+FMN_SCREENW_TILES) continue;
    if (poi->y>=fmn_vy+FMN_SCREENH_TILES) continue;
    void (*fn)(uint8_t,uint8_t,uint8_t,uint8_t)=poi->qp;
    fn(state,poi->q[1],poi->q[2],poi->q[3]);
  }
}

/* Add PROXIMITY POI currently visible.
 */
 
void fmn_map_add_proximity_pois() {
  if (!fmn_map) return;
  const struct fmn_map_poi *poi=fmn_map->poiv;
  uint16_t i=fmn_map->poic;
  for (;i-->0;poi++) {
    if (poi->q[0]!=FMN_POI_PROXIMITY) continue;
    if (poi->x<fmn_vx) continue;
    if (poi->y<fmn_vy) continue;
    if (poi->x>=fmn_vx+FMN_SCREENW_TILES) continue;
    if (poi->y>=fmn_vy+FMN_SCREENH_TILES) continue;
    fmn_proximity_add(
      poi->x*FMN_MM_PER_TILE,
      poi->y*FMN_MM_PER_TILE,
      poi->q[1],poi->q[2],poi->q[3],
      poi->qp
    );
  }
}

/* Find edge doors.
 */
 
struct fmn_map_edge_door_context {
  const struct fmn_map *from;
  int16_t vx,vy;
  int16_t *dx,*dy;
  struct fmn_map *to;
};

static void fmn_map_find_edge_door(
  struct fmn_map_edge_door_context *ctx,
  uint8_t poix,uint8_t poiy,
  uint8_t (*cb)(struct fmn_map_edge_door_context *ctx,const struct fmn_map_poi *poi)
) {
  const struct fmn_map_poi *poi=ctx->from->poiv;
  uint16_t i=ctx->from->poic;
  for (;i-->0;poi++) {
    if (poi->q[0]!=FMN_POI_EDGE_DOOR) continue;
    if (poi->x!=poix) continue;
    if (poi->y!=poiy) continue;
    if (cb(ctx,poi)) {
      ctx->to=poi->qp;
      return;
    }
  }
}

static uint8_t fmn_map_match_edge_door_left(struct fmn_map_edge_door_context *ctx,const struct fmn_map_poi *poi) {
  int16_t d=(poi->q[1]<<8)|poi->q[2];
  int16_t vy=ctx->vy-d;
  if (vy<0) return 0;
  const struct fmn_map *to=poi->qp;
  if (vy>=to->h) return 0;
  *(ctx->dx)=to->w*FMN_MM_PER_TILE;
  *(ctx->dy)=-d*FMN_MM_PER_TILE;
  return 1;
}

static uint8_t fmn_map_match_edge_door_right(struct fmn_map_edge_door_context *ctx,const struct fmn_map_poi *poi) {
  int16_t d=(poi->q[1]<<8)|poi->q[2];
  int16_t vy=ctx->vy-d;
  if (vy<0) return 0;
  const struct fmn_map *to=poi->qp;
  if (vy>=to->h) return 0;
  *(ctx->dx)=-ctx->from->w*FMN_MM_PER_TILE;
  *(ctx->dy)=-d*FMN_MM_PER_TILE;
  return 1;
}

static uint8_t fmn_map_match_edge_door_up(struct fmn_map_edge_door_context *ctx,const struct fmn_map_poi *poi) {
  int16_t d=(poi->q[1]<<8)|poi->q[2];
  int16_t vx=ctx->vx-d;
  if (vx<0) return 0;
  const struct fmn_map *to=poi->qp;
  if (vx>=to->w) return 0;
  *(ctx->dx)=-d*FMN_MM_PER_TILE;
  *(ctx->dy)=to->h*FMN_MM_PER_TILE;
  return 1;
}

static uint8_t fmn_map_match_edge_door_down(struct fmn_map_edge_door_context *ctx,const struct fmn_map_poi *poi) {
  int16_t d=(poi->q[1]<<8)|poi->q[2];
  int16_t vx=ctx->vx-d;
  if (vx<0) return 0;
  const struct fmn_map *to=poi->qp;
  if (vx>=to->w) return 0;
  *(ctx->dx)=-d*FMN_MM_PER_TILE;
  *(ctx->dy)=-ctx->from->h*FMN_MM_PER_TILE;
  return 1;
}
 
struct fmn_map *fmn_map_find_edge_door_left(const struct fmn_map *from,int16_t vytiles,int16_t *dxmm,int16_t *dymm) {
  struct fmn_map_edge_door_context ctx={from,0,vytiles,dxmm,dymm};
  fmn_map_find_edge_door(&ctx,0,from->h-1,fmn_map_match_edge_door_left);
  return ctx.to;
}

struct fmn_map *fmn_map_find_edge_door_right(const struct fmn_map *from,int16_t vytiles,int16_t *dxmm,int16_t *dymm) {
  struct fmn_map_edge_door_context ctx={from,0,vytiles,dxmm,dymm};
  fmn_map_find_edge_door(&ctx,from->w-1,0,fmn_map_match_edge_door_right);
  return ctx.to;
}

struct fmn_map *fmn_map_find_edge_door_up(const struct fmn_map *from,int16_t vxtiles,int16_t *dxmm,int16_t *dymm) {
  struct fmn_map_edge_door_context ctx={from,vxtiles,0,dxmm,dymm};
  fmn_map_find_edge_door(&ctx,0,0,fmn_map_match_edge_door_up);
  return ctx.to;
}

struct fmn_map *fmn_map_find_edge_door_down(const struct fmn_map *from,int16_t vxtiles,int16_t *dxmm,int16_t *dymm) {
  struct fmn_map_edge_door_context ctx={from,vxtiles,0,dxmm,dymm};
  fmn_map_find_edge_door(&ctx,from->w-1,from->h-1,fmn_map_match_edge_door_down);
  return ctx.to;
}

/* Search POI by location. Always returns a valid position in map->poiv, possibly the very end.
 * If any POI exist at this exact location, we return the first match (lowest index).
 */
 
static uint16_t fmn_map_poi_search(const struct fmn_map *map,uint8_t x,uint8_t y) {
  uint16_t lo=0,hi=map->poic;
  while (lo<hi) {
    uint16_t ck=(lo+hi)>>1;
    const struct fmn_map_poi *poi=map->poiv+ck;
         if (y<poi->y) hi=ck;
    else if (y>poi->y) lo=ck+1;
    else if (x<poi->x) hi=ck;
    else if (x>poi->x) lo=ck+1;
    else {
      while ((ck>lo)&&(poi[-1].x==x)&&(poi[-1].y==y)) { ck--; poi--; }
      return ck;
    }
  }
  return lo;
}

/* Iterate POI.
 */
 
int8_t fmn_map_for_each_poi(
  uint8_t x,uint8_t y,uint8_t w,uint8_t h,
  int8_t (*cb)(const struct fmn_map_poi *poi,void *userdata),
  void *userdata
) {
  if (!fmn_map) return 0;
  uint16_t p=fmn_map_poi_search(fmn_map,x,y);
  const struct fmn_map_poi *poi=fmn_map->poiv+p;
  for (;(p<fmn_map->poic)&&(poi->y<y+h);poi++,p++) {
    if (poi->x<x) continue;
    if (poi->x>=x+w) continue;
    int8_t err=cb(poi,userdata);
    if (err) return err;
  }
  return 0;
}

/* Get region.
 */

uint8_t fmn_map_get_region() {
  if (fmn_map) return fmn_map->region;
  return 0;
}
