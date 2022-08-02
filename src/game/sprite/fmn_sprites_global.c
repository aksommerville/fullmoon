#include "game/fullmoon.h"
#include "game/model/fmn_map.h"
#include "game/sprite/fmn_sprite.h"

/* Misc globals.
 */
 
static int8_t fmn_sprite_sort_dir=1;

/* Load.
 */
 
static int8_t fmn_sprites_load_1(const struct fmn_map_poi *poi,void *userdata) {
  if (poi->q[0]!=FMN_POI_SPRITE) return 0;
  int16_t xmm=poi->x*FMN_MM_PER_TILE+(FMN_MM_PER_TILE>>1);
  int16_t ymm=poi->y*FMN_MM_PER_TILE+(FMN_MM_PER_TILE>>1);
  struct fmn_sprite *sprite=fmn_sprite_new(0,poi->qp,xmm,ymm,poi->q[1],poi->q[2],poi->q[3]);
  if (!sprite) {
    return 0;
  }
  return 0;
}
 
void fmn_sprites_load() {
  fmn_map_for_each_poi(0,0,0xff,0xff,fmn_sprites_load_1,0);
  
  uint8_t herotx,heroty;
  fmn_map_get_init_position(&herotx,&heroty);
  int16_t herox=herotx*FMN_MM_PER_TILE+(FMN_MM_PER_TILE>>1);
  int16_t heroy=heroty*FMN_MM_PER_TILE+(FMN_MM_PER_TILE>>1);
  struct fmn_sprite *hero=fmn_sprite_new(&fmn_sprtype_hero,0,herox,heroy,0,0,0);
}

/* Update.
 */
 
void fmn_sprites_update() {
  struct fmn_sprite **p=fmn_spritev;
  uint16_t i=fmn_spritec;
  for (;i-->0;p++) {
    struct fmn_sprite *sprite=*p;
    if (!sprite->type->update) continue;
    sprite->type->update(sprite);
  }
}

/* Compare sprites for render sort.
 */
 
static int8_t fmn_sprite_rendercmp(const struct fmn_sprite *a,const struct fmn_sprite *b) {
  if (a->layer<b->layer) return -1;
  if (a->layer>b->layer) return 1;
  if (a->y+a->h<b->y+b->h) return -1;
  if (a->y+a->h>b->y+b->h) return 1;
  return 0;
}

/* Render.
 */
 
void fmn_sprites_render(struct fmn_image *fb) {
  
  // First run one pass of a cocktail sort to nudge them into order.
  if (fmn_spritec>1) {
    uint16_t first,last,i;
    if (fmn_sprite_sort_dir==1) {
      first=0;
      last=fmn_spritec-1;
    } else {
      first=fmn_spritec-1;
      last=0;
    }
    for (i=first;i!=last;i+=fmn_sprite_sort_dir) {
      if (fmn_sprite_rendercmp(fmn_spritev[i],fmn_spritev[i+fmn_sprite_sort_dir])==fmn_sprite_sort_dir) {
        struct fmn_sprite *tmp=fmn_spritev[i];
        fmn_spritev[i]=fmn_spritev[i+fmn_sprite_sort_dir];
        fmn_spritev[i+fmn_sprite_sort_dir]=tmp;
      }
    }
    fmn_sprite_sort_dir*=-1;
  }
  
  // Render in order.
  int16_t scrollx,scrolly;
  fmn_map_get_scroll_mm(&scrollx,&scrolly);
  int16_t scrright=scrollx+FMN_SCREENW_MM;
  int16_t scrbottom=scrolly+FMN_SCREENH_MM;
  struct fmn_sprite **p=fmn_spritev;
  uint16_t i=fmn_spritec;
  for (;i-->0;p++) {
    struct fmn_sprite *sprite=*p;
    if (sprite->x>=scrright) continue;
    if (sprite->y>=scrbottom) continue;
    if (sprite->x+sprite->w<=scrollx) continue;
    if (sprite->y+sprite->h<=scrolly) continue;
    sprite->type->render(fb,scrollx,scrolly,sprite);
  }
}
