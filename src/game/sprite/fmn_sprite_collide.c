#include "game/fullmoon.h"
#include "game/sprite/fmn_sprite.h"
#include "game/model/fmn_map.h"

#define FMN_COLLIDE_MARGIN FMN_MM_PER_TILE
#define FMN_SOLID_LIMIT 16

/* Collision detection context.
 */
 
struct fmn_collide {
  struct fmn_sprite *sprite;
  int16_t x,y,w,h; // workspace bounds
  uint8_t collision; // nonzero if a collision detected in initial sweep
  const struct fmn_map *map;
  struct fmn_solid {
    int16_t x,y,w,h;
    struct fmn_sprite *owner;
  } solidv[FMN_SOLID_LIMIT];
  uint8_t solidc;
};

/* Nonzero if the grid under this rectangle contains only cells with the given properties.
 */
 
static uint8_t fmn_collide_fully_solid(
  const struct fmn_collide *collide,
  int16_t x,int16_t y,int16_t w,int16_t h,
  uint8_t cellprops
) {
  int16_t cola=x/FMN_MM_PER_TILE;
  int16_t colz=(x+w-1)/FMN_MM_PER_TILE;
  if (cola<0) cola=0;
  if (colz>=collide->map->w) colz=collide->map->w-1;
  if (cola>colz) return 0;
  int16_t rowa=y/FMN_MM_PER_TILE;
  int16_t rowz=(y+h-1)/FMN_MM_PER_TILE;
  if (rowa<0) rowa=0;
  if (rowz>=collide->map->h) rowz=collide->map->h-1;
  if (rowa>rowz) return 0;
  const uint8_t *cellrow=collide->map->v+rowa*collide->map->w+cola;
  int16_t row=rowa; for (;row<=rowz;row++,cellrow+=collide->map->w) {
    const uint8_t *cellp=cellrow;
    int16_t col=cola; for (;col<=colz;col++,cellp++) {
      if (!(collide->map->tileprops[*cellp]&cellprops)) return 0;
    }
  }
  return 1;
}

/* Bring the walls of the workspace in where they are spanned by solid grid cells.
 * This is not technically necessary but it's easy and it reduces our search space for later steps.
 * Returns nonzero if the space is still wide enough to be solvable.
 */
 
static uint8_t fmn_collide_collapse_walls(struct fmn_collide *collide,uint8_t cellprops) {
  while (cellprops&&(collide->w>0)&&(collide->h>0)) {
    uint8_t done=1;
    if (fmn_collide_fully_solid(collide,collide->x,collide->y,1,collide->h,cellprops)) {
      done=0;
      int16_t rm=collide->x%FMN_MM_PER_TILE;
      if (rm<0) rm+=FMN_MM_PER_TILE;
      else rm=FMN_MM_PER_TILE-rm;
      collide->x+=rm;
      collide->w-=rm;
    }
    if (fmn_collide_fully_solid(collide,collide->x,collide->y,collide->w,1,cellprops)) {
      done=0;
      int16_t rm=collide->y%FMN_MM_PER_TILE;
      if (rm<0) rm+=FMN_MM_PER_TILE;
      else rm=FMN_MM_PER_TILE-rm;
      collide->y+=rm;
      collide->h-=rm;
    }
    if (fmn_collide_fully_solid(collide,collide->x+collide->w-1,collide->y,1,collide->h,cellprops)) {
      done=0;
      int16_t rm=(collide->x+collide->w)%FMN_MM_PER_TILE;
      if (rm<=0) rm+=FMN_MM_PER_TILE;
      collide->w-=rm;
    }
    if (fmn_collide_fully_solid(collide,collide->x,collide->y+collide->h-1,collide->w,1,cellprops)) {
      done=0;
      int16_t rm=(collide->y+collide->h)%FMN_MM_PER_TILE;
      if (rm<=0) rm+=FMN_MM_PER_TILE;
      collide->h-=rm;
    }
    if (done) break;
  }
  if (collide->w<collide->sprite->w) return 0;
  if (collide->h<collide->sprite->h) return 0;
  return 1;
}

/* Add a solid if we have room for it.
 * Also check against the sprite, record whether a collision exists.
 */
 
static void fmn_collide_add_solid(struct fmn_collide *collide,int16_t x,int16_t y,int16_t w,int16_t h,struct fmn_sprite *owner) {
  if (!collide->collision) {
    if (
      (x+w>collide->sprite->x)&&
      (y+h>collide->sprite->y)&&
      (x<collide->sprite->x+collide->sprite->w)&&
      (y<collide->sprite->y+collide->sprite->h)
    ) collide->collision=1;
  }
  if (collide->solidc>=FMN_SOLID_LIMIT) return;
  struct fmn_solid *solid=collide->solidv+collide->solidc++;
  solid->x=x;
  solid->y=y;
  solid->w=w;
  solid->h=h;
  solid->owner=owner;
}

/* Find solid grid cells touching the workspace.
 */
 
static void fmn_collide_find_grid_cells(struct fmn_collide *collide,uint8_t cellprops) {
  int16_t cola=collide->x/FMN_MM_PER_TILE;
  int16_t colz=(collide->x+collide->w-1)/FMN_MM_PER_TILE;
  if (cola<0) cola=0;
  if (colz>=collide->map->w) colz=collide->map->w-1;
  if (cola>colz) return;
  int16_t rowa=collide->y/FMN_MM_PER_TILE;
  int16_t rowz=(collide->y+collide->h-1)/FMN_MM_PER_TILE;
  if (rowa<0) rowa=0;
  if (rowz>=collide->map->h) rowz=collide->map->h-1;
  if (rowa>rowz) return;
  const uint8_t *cellrow=collide->map->v+rowa*collide->map->w+cola;
  int16_t row=rowa; for (;row<=rowz;row++,cellrow+=collide->map->w) {
    const uint8_t *cellp=cellrow;
    uint8_t runc=0;
    int16_t runx;
    int16_t col=cola; for (;col<=colz;col++,cellp++) {
      if (collide->map->tileprops[*cellp]&cellprops) {
        if (!runc++) runx=col*FMN_MM_PER_TILE;
      } else if (runc) {
        fmn_collide_add_solid(collide,runx,row*FMN_MM_PER_TILE,runc*FMN_MM_PER_TILE,FMN_MM_PER_TILE,0);
        runc=0;
      }
    }
    if (runc) {
      fmn_collide_add_solid(collide,runx,row*FMN_MM_PER_TILE,runc*FMN_MM_PER_TILE,FMN_MM_PER_TILE,0);
    }
  }
}

/* Find solid sprites touching the workspace.
 */
 
static void fmn_collide_find_sprites(struct fmn_collide *collide,uint8_t spriteflags) {
  int16_t right=collide->x+collide->w;
  int16_t bottom=collide->y+collide->h;
  struct fmn_sprite **p=fmn_spritev;
  uint16_t i=fmn_spritec;
  for (;i-->0;p++) {
    struct fmn_sprite *sprite=*p;
    if (!(sprite->flags&spriteflags)) continue;
    if (sprite->x>=right) continue;
    if (sprite->y>=bottom) continue;
    if (sprite->x+sprite->w<=collide->x) continue;
    if (sprite->y+sprite->h<=collide->y) continue;
    if (sprite==collide->sprite) continue;
    fmn_collide_add_solid(collide,sprite->x,sprite->y,sprite->w,sprite->h,sprite);
  }
}

/* Populate (solidv) with every matching sprite and grid cell we can find.
 */
 
static void fmn_collide_find_solids(struct fmn_collide *collide,uint8_t cellprops,uint8_t spriteflags) {
  if (cellprops) fmn_collide_find_grid_cells(collide,cellprops);
  if (spriteflags) fmn_collide_find_sprites(collide,spriteflags);
}

/* Any solid with an adjacent gap too narrow for the sprite to fit in, expand to fill it.
 * TODO is this useful?
 */
 
static void fmn_collide_expand_solids(struct fmn_collide *collide) {
  struct fmn_solid *solid=collide->solidv;
  uint8_t i=collide->solidc;
  for (;i-->0;solid++) {
    int16_t n;
    if ((n=solid->x-collide->x)<collide->sprite->w) {
      solid->x=collide->x;
      solid->w+=n;
    }
    if ((n=solid->y-collide->y)<collide->sprite->h) {
      solid->y=collide->y;
      solid->h+=n;
    }
    if ((n=collide->x+collide->w-solid->w-solid->x)<collide->sprite->w) {
      solid->w=collide->x+collide->w-solid->x;
    }
    if ((n=collide->y+collide->h-solid->h-solid->y)<collide->sprite->h) {
      solid->h=collide->y+collide->h-solid->y;
    }
  }
}

/* Eliminate any solid contained by another.
 */
 
static void fmn_collide_cull_solids(struct fmn_collide *collide) {
  //TODO
}

/* Find the best available spot to put the sprite in.
 * This is the main event for collision detection.
 * (freedom) is some combination of (FMN_DIR_N|S|E|W), the directions we are allowed to move.
 * Return nonzero if we put (sprite) in a valid position, or zero if unsolvable.
 */
 
static uint8_t fmn_collide_find_best_escape(struct fmn_collide *collide,uint8_t freedom) {
  if (!freedom) return 0;
  struct fmn_solid *solid=collide->solidv;
  uint8_t i=collide->solidc;
  for (;i-->0;solid++) {
  
    // Find the straight escapement in each direction.
    // Any <=0, this solid does not touch the sprite.
    int16_t escr=solid->x+solid->w-collide->sprite->x;
    if (escr<=0) continue;
    int16_t escd=solid->y+solid->h-collide->sprite->y;
    if (escd<=0) continue;
    int16_t escl=collide->sprite->x+collide->sprite->w-solid->x;
    if (escl<=0) continue;
    int16_t escu=collide->sprite->y+collide->sprite->h-solid->y;
    if (escu<=0) continue;
    
    // Record the four possible escapes, order doesn't matter yet.
    struct fmn_escape {
      int16_t d;
      int8_t dx,dy; // -1,0,1
      uint8_t bit; // eg FMN_DIR_N
    } escv[4];
    uint8_t escc=0;
    if (freedom&FMN_DIR_W) escv[escc++]=(struct fmn_escape){escl,-1,0,FMN_DIR_E};
    if (freedom&FMN_DIR_E) escv[escc++]=(struct fmn_escape){escr,1,0,FMN_DIR_W};
    if (freedom&FMN_DIR_N) escv[escc++]=(struct fmn_escape){escu,0,-1,FMN_DIR_S};
    if (freedom&FMN_DIR_S) escv[escc++]=(struct fmn_escape){escd,0,1,FMN_DIR_N};
    
    // Bubble-sort the escapes short to long.
    #define COMPARE(ap,bp) if ((bp<escc)&&(escv[ap].d>escv[bp].d)) { \
      struct fmn_escape tmp=escv[ap]; \
      escv[ap]=escv[bp]; \
      escv[bp]=tmp; \
    }
    COMPARE(0,1)
    COMPARE(1,2)
    COMPARE(2,3)
    COMPARE(1,2)
    COMPARE(0,1)
    COMPARE(1,2)
    #undef COMPARE
    
    // Apply each escape and recur with one bit stripped from (freedom).
    // First thing to return OK, keep it.
    uint8_t i=0; for (;i<escc;i++) {
      collide->sprite->x+=escv[i].d*escv[i].dx;
      collide->sprite->y+=escv[i].d*escv[i].dy;
      if (
        (collide->sprite->x>=collide->x)&&
        (collide->sprite->y>=collide->y)&&
        (collide->sprite->x+collide->sprite->w<=collide->x+collide->w)&&
        (collide->sprite->y+collide->sprite->h<=collide->y+collide->h)
      ) {
        if (fmn_collide_find_best_escape(collide,freedom&~escv[i].bit)) {
          return 1;
        }
      }
      collide->sprite->x-=escv[i].d*escv[i].dx;
      collide->sprite->y-=escv[i].d*escv[i].dy;
    }
    
    // No escaped recurrence yielded a valid position. Report unsolvable.
    return 0;
  }
  // If we end up here, nothing collided so we're good.
  return 1;
}

/* Check collisions for one sprite, main entry point.
 */
 
uint8_t fmn_sprite_collide(
  int16_t *adjx,int16_t *adjy,
  struct fmn_sprite *sprite,
  uint8_t cellprops,
  uint8_t spriteflags,
  uint8_t resolve
) {
  if (!sprite) return 0;
  struct fmn_collide collide={
    .sprite=sprite,
    .x=sprite->x-FMN_COLLIDE_MARGIN,
    .y=sprite->y-FMN_COLLIDE_MARGIN,
    .w=sprite->w+(FMN_COLLIDE_MARGIN<<1),
    .h=sprite->h+(FMN_COLLIDE_MARGIN<<1),
  };
  
  if (cellprops) {
    if (collide.map=fmn_map_get()) {
      if (!collide.map->tileprops) {
        collide.map=0;
        cellprops=0;
      }
    } else {
      cellprops=0;
    }
  }
  
  // Bring in the workspace walls and terminate if it becomes definitely unsolvable.
  if (!fmn_collide_collapse_walls(&collide,cellprops)) {
    if (adjx) *adjx=0;
    if (adjy) *adjy=0;
    return 1;
  }
  
  // If they are only looking to detect, and not resolve or report, we might be able to give a positive already due to collapse.
  if (!adjx&&!resolve) {
    if (
      (sprite->x<collide.x)||
      (sprite->y<collide.y)||
      (sprite->x+sprite->w>collide.x+collide.w)||
      (sprite->y+sprite->h>collide.y+collide.h)
    ) {
      return 1;
    }
  }
  
  // Force the sprite to within the workspace bounds.
  // Record its virgin position, and we'll use the sprite's live position as scratch.
  uint8_t freedom=FMN_DIR_N|FMN_DIR_S|FMN_DIR_W|FMN_DIR_E;
  int16_t ox=sprite->x,oy=sprite->y;
  if (sprite->x<collide.x) { collide.collision=1; sprite->x=collide.x; freedom&=~FMN_DIR_W; }
  if (sprite->y<collide.y) { collide.collision=1; sprite->y=collide.y; freedom&=~FMN_DIR_N; }
  if (sprite->x+sprite->w>collide.x+collide.w) { collide.collision=1; sprite->x=collide.x+collide.w-sprite->w; freedom&=~FMN_DIR_E; }
  if (sprite->y+sprite->h>collide.y+collide.h) { collide.collision=1; sprite->y=collide.y+collide.h-sprite->h; freedom&=~FMN_DIR_S; }
  
  // Find all the sprites and solid cells.
  fmn_collide_find_solids(&collide,cellprops,spriteflags);
  
  // If they only want to detect, or if there's nothing to detect, we have the final answer.
  if (!adjx&&!resolve) {
    sprite->x=ox;
    sprite->y=oy;
    return collide.collision;
  } else if (!collide.collision) {
    return 0;
  }
  
  // Expand and cull.
  fmn_collide_expand_solids(&collide);
  fmn_collide_cull_solids(&collide);
  
  // Resolve iteratively until we find a good position.
  if (fmn_collide_find_best_escape(&collide,freedom)) {
    if (adjx) *adjx=sprite->x-ox;
    if (adjy) *adjy=sprite->y-oy;
    if (!resolve) {
      sprite->x=ox;
      sprite->y=oy;
    }
    return 1;
  }
  sprite->x=ox;
  sprite->y=oy;
  if (adjx) *adjx=0;
  if (adjy) *adjy=0;
  return 1;
}
