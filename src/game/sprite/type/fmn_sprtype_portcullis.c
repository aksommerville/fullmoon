#include "game/fullmoon.h"
#include "game/sprite/fmn_sprite.h"

#define opening sprite->bv[3]
#define openness sprite->sv[0]

#define FMN_PORTCULLIS_OPEN_SPEED 1
#define FMN_PORTCULLIS_OPEN_MAX ((FMN_MM_PER_TILE*15)/8)

/* Setup.
 */
 
static int8_t _portcullis_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  sprite->x-=FMN_MM_PER_TILE>>1;
  sprite->y-=FMN_MM_PER_TILE>>1;
  sprite->w=FMN_MM_PER_TILE*3;
  sprite->h=FMN_MM_PER_TILE*2;
  return 0;
}

/* Update.
 */
 
static void _portcullis_update(struct fmn_sprite *sprite) {
  if (!opening) return;
  if (openness>=FMN_PORTCULLIS_OPEN_MAX) return;
  openness+=FMN_PORTCULLIS_OPEN_SPEED;
}

/* Open.
 */
 
static void _portcullis_spell_open(struct fmn_sprite *sprite) {
  sprite->flags&=~FMN_SPRITE_FLAG_SOLID;
  opening=1;
}

/* Render.
 */
 
static void _portcullis_render(
  struct fmn_image *dst,
  int16_t scrollx,int16_t scrolly,
  struct fmn_sprite *sprite
) {
  int16_t dstx=((sprite->x-scrollx)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  int16_t dsty=((sprite->y-scrolly)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  int16_t srcx=(sprite->tileid&0x0f)*FMN_TILESIZE;
  int16_t srcy=(sprite->tileid>>4)*FMN_TILESIZE;
  int16_t srcw=FMN_TILESIZE*3;
  int16_t srch=FMN_TILESIZE*2;
  if (openness) {
    int16_t adj=(openness*FMN_TILESIZE)/FMN_MM_PER_TILE;
    srcy+=adj;
    srch-=adj;
  }
  fmn_blit(dst,dstx,dsty,sprite->image,srcx,srcy,srcw,srch,0);
}

/* Type definition.
 */
 
const struct fmn_sprtype fmn_sprtype_portcullis={
  .name="portcullis",
  .init=_portcullis_init,
  .update=_portcullis_update,
  .render=_portcullis_render,
  .spell_open=_portcullis_spell_open,
};
