#include "game/fullmoon.h"
#include "fmn_sprite.h"
#include <string.h>

/* Globals.
 */
 
struct fmn_sprite *fmn_spritev[FMN_SPRITE_LIMIT];
uint16_t fmn_spritec=0;

static struct fmn_sprite fmn_sprite_storage[FMN_SPRITE_LIMIT]={0};
static uint16_t fmn_sprite_storage_next=0; // unused slot, or >=LIMIT

/* Delete.
 */
 
void fmn_sprite_del(struct fmn_sprite *sprite) {
  if (!sprite||!sprite->type) return;
  uint32_t p=sprite-fmn_sprite_storage;
  if (p>=FMN_SPRITE_LIMIT) return;
  if (sprite->type->del) sprite->type->del(sprite);
  sprite->type=0;
  uint16_t i=fmn_spritec;
  struct fmn_sprite **q=fmn_spritev+i-1;
  while (i-->0) {
    if (*q==sprite) {
      fmn_spritec--;
      memmove(q,q+1,sizeof(void*)*(fmn_spritec-i));
      return;
    }
  }
}

/* Allocate.
 */

struct fmn_sprite *fmn_sprite_alloc() {
  if (fmn_sprite_storage_next>=FMN_SPRITE_LIMIT) {
    fmn_sprite_storage_next=0;
    while (
      (fmn_sprite_storage_next<FMN_SPRITE_LIMIT)&&
      fmn_sprite_storage[fmn_sprite_storage_next].type
    ) fmn_sprite_storage_next++;
    if (fmn_sprite_storage_next>=FMN_SPRITE_LIMIT) return 0;
  }
  struct fmn_sprite *sprite=fmn_sprite_storage+fmn_sprite_storage_next;
  fmn_sprite_storage_next++;
  while (
    (fmn_sprite_storage_next<FMN_SPRITE_LIMIT)&&
    fmn_sprite_storage[fmn_sprite_storage_next].type
  ) fmn_sprite_storage_next++;
  fmn_spritev[fmn_spritec++]=sprite;
  return sprite;
}

/* Clear.
 */
 
void fmn_sprites_clear() {
  while (fmn_spritec>0) {
    fmn_spritec--;
    fmn_sprite_del(fmn_spritev[fmn_spritec]);
  }
}
