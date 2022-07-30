#include "game/fullmoon.h"
#include "game/model/fmn_hero.h"
#include "game/sprite/fmn_sprite.h"

/* tileid is one of (0,0x90,0x94,0x98,0x9a).
 * tileid zero means invalid.
 * tileid does not change during animation.
 */
#define treasure_mask sprite->bv[0]
#define timer sprite->bv[4]
#define timer_period sprite->bv[5]

static int8_t _fmn_treasure_init(struct fmn_sprite *sprite,const struct fmn_sprdef *def) {
  if (fmn_game_generate_password()&treasure_mask) {
    fmn_sprite_del_later(sprite);
    return 0;
  }
  switch (treasure_mask) {
    case FMN_STATE_BROOM: {
        sprite->tileid=0x98;
        timer_period=60;
        sprite->w=FMN_MM_PER_TILE*2;
        sprite->h=FMN_MM_PER_TILE;
      } break;
    case FMN_STATE_FEATHER: {
        sprite->tileid=0x90;
        timer_period=60;
        sprite->w=FMN_MM_PER_TILE;
        sprite->h=FMN_MM_PER_TILE;
      } break;
    case FMN_STATE_WAND: {
        sprite->tileid=0x94;
        timer_period=60;
        sprite->w=FMN_MM_PER_TILE;
        sprite->h=FMN_MM_PER_TILE;
      } break;
    case FMN_STATE_UMBRELLA: {
        sprite->tileid=0x9a;
        timer_period=60;
        sprite->w=FMN_MM_PER_TILE*2;
        sprite->h=FMN_MM_PER_TILE;
      } break;
    default: sprite->tileid=0;
  }
  sprite->x-=sprite->w>>1;
  sprite->y-=sprite->h>>1;
  return 0;
}

static void _fmn_treasure_update(struct fmn_sprite *sprite) {
  if (!sprite->tileid) {
    fmn_sprite_del_later(sprite);
    return;
  }
  timer++;
  if (timer>=timer_period) {
    timer=0;
  }
  int16_t herox,heroy;
  fmn_hero_get_world_position_center(&herox,&heroy);
  if ((herox>=sprite->x)&&(heroy>=sprite->y)&&(herox<sprite->x+sprite->w)&&(heroy<sprite->y+sprite->h)) {
    fmn_game_set_state(treasure_mask,treasure_mask);
    fmn_sprite_del_later(sprite);
  }
}

static void _fmn_treasure_render(
  struct fmn_image *dst,
  int16_t xscrollmm,int16_t yscrollmm,
  struct fmn_sprite *sprite
) {
  if (!sprite->tileid) return;
  int16_t dstx=((sprite->x-xscrollmm)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  int16_t dsty=((sprite->y-yscrollmm)*FMN_TILESIZE)/FMN_MM_PER_TILE;
  switch (sprite->tileid) {
  
    case 0x90: { // Feather: 4 frames and small shadow.
        if (timer&1) {
          fmn_blit_tile(dst,dstx,dsty,sprite->image,0x96+timer/30,0);
        }
        fmn_blit_tile(dst,dstx,dsty,sprite->image,0x90+timer/15,0);
      } break;
      
    case 0x94: { // Wand: 2 frames and small shadow.
        #if FMN_GFXSCALE==1
          int16_t floaty=dsty+(timer*FMN_GFXSCALE)/30;
        #else
          int16_t floaty=dsty;
          int16_t offset=(timer*FMN_GFXSCALE)/30;
          if (timer>=30) offset=2*FMN_GFXSCALE-offset;
          floaty+=offset;
        #endif
        if (timer&1) {
          fmn_blit_tile(dst,dstx,dsty,sprite->image,0x96+timer/30,0);
        }
        fmn_blit_tile(dst,dstx,floaty,sprite->image,0x94+((timer/15)&1),0);
      } break;
      
    case 0x98:
    case 0x9a: { // Broom and umbrella: 1 frame moving and large shadow.
        #if FMN_GFXSCALE==1
          int16_t floaty=dsty-2*FMN_GFXSCALE+(timer*FMN_GFXSCALE)/30;
        #else
          int16_t floaty=dsty-2*FMN_GFXSCALE;
          int16_t offset=(timer*FMN_GFXSCALE)/15;
          if (timer>=30) offset=4*FMN_GFXSCALE-offset;
          floaty+=offset;
        #endif
        if (timer&1) {
          uint8_t shadowtile=0x9c+timer/30;
          fmn_blit_tile(dst,dstx,dsty,sprite->image,shadowtile,0);
          fmn_blit_tile(dst,dstx+FMN_TILESIZE,dsty,sprite->image,shadowtile,FMN_XFORM_XREV);
        }
        fmn_blit_tile(dst,dstx,floaty,sprite->image,sprite->tileid,0);
        fmn_blit_tile(dst,dstx+FMN_TILESIZE,floaty,sprite->image,sprite->tileid+1,0);
      } break;
      
  }
}

const struct fmn_sprtype fmn_sprtype_treasure={
  .name="treasure",
  .init=_fmn_treasure_init,
  .update=_fmn_treasure_update,
  .render=_fmn_treasure_render,
  .hitbox=fmn_sprite_hitbox_none,
};
