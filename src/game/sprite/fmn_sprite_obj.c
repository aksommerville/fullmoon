#include "game/fullmoon.h"
#include "game/sprite/fmn_sprite.h"

/* New sprite.
 */
 
struct fmn_sprite *fmn_sprite_new(
  const struct fmn_sprtype *type,
  const struct fmn_sprdef *def,
  int16_t xmm,int16_t ymm,
  uint8_t q1,uint8_t q2,uint8_t q3
) {
  if (!type&&!def) return 0;
  if (type&&def&&(def->type!=type)) return 0;
  if (!type) type=def->type;
  
  struct fmn_sprite *sprite=fmn_sprite_alloc();
  if (!sprite) return 0;
  
  sprite->type=type;
  sprite->x=xmm;
  sprite->y=ymm;
  sprite->bv[0]=q1;
  sprite->bv[1]=q2;
  sprite->bv[2]=q3;
  
  if (def) {
    sprite->image=def->image;
    sprite->tileid=def->tileid;
    sprite->xform=def->xform;
  }
  
  if (type->init&&(type->init(sprite,def)<0)) {
    fmn_sprite_del(sprite);
    return 0;
  }
  
  return sprite;
}

/* Default render.
 */

void fmn_sprite_render_default(struct fmn_image *dst,int16_t xscroll,int16_t yscroll,struct fmn_sprite *sprite) {
  if (!sprite||!sprite->image) return;
  fmn_blit_tile(dst,
    (sprite->x+(sprite->w>>1)-xscroll)/FMN_MM_PER_PIXEL-(FMN_TILESIZE>>1),
    (sprite->y+(sprite->h>>1)-yscroll)/FMN_MM_PER_PIXEL-(FMN_TILESIZE>>1),
    sprite->image,sprite->tileid,sprite->xform
  );
}

/* Default hitbox.
 */

void fmn_sprite_hitbox_none(int16_t *rx,int16_t *ry,int16_t *w,int16_t *h,struct fmn_sprite *sprite) {
  *w=*h=0;
}

void fmn_sprite_hitbox_all(int16_t *rx,int16_t *ry,int16_t *w,int16_t *h,struct fmn_sprite *sprite) {
  *rx=*ry=0;
  *w=sprite->w;
  *h=sprite->h;
}
