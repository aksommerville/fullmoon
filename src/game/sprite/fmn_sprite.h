/* fmn_sprite.h
 */
 
#ifndef FMN_SPRITE_H
#define FMN_SPRITE_H

/* Actions on the global set of all sprites.
 *****************************************************************/

void fmn_sprites_clear();
void fmn_sprites_load();
void fmn_sprites_update();
void fmn_sprites_render(struct fmn_image *fb);

/* Sprite instance.
 * They are stored in a statically-allocated list, so there's a global limit on sprite count.
 **************************************************************/
 
#define FMN_SPRITE_LIMIT 128
#define FMN_SPRITE_BV_COUNT 8
#define FMN_SPRITE_SV_COUNT 8
#define FMN_SPRITE_PV_COUNT 8
 
struct fmn_sprite {
  const struct fmn_sprtype *type;
  int16_t x,y,w,h; // mm relative to map. Must contain both visual and physical bounds.
  int8_t layer; // for render sorting
// (type->render) may choose to ignore these, but most will use:
  const struct fmn_image *image;
  uint8_t tileid;
  uint8_t xform;
// Loose data for type's use:
  uint8_t bv[FMN_SPRITE_BV_COUNT]; // [0,1,2] initially populated from spawn point
  int16_t sv[FMN_SPRITE_SV_COUNT];
  void *pv[FMN_SPRITE_PV_COUNT];
};

extern struct fmn_sprite *fmn_spritev[FMN_SPRITE_LIMIT];
extern uint16_t fmn_spritec; // READONLY

/* This will clean up the sprite and remove it from the global list.
 */
void fmn_sprite_del(struct fmn_sprite *sprite);

/* Create a new sprite centered at (xmm,ymm) and add it to the global list.
 */
struct fmn_sprite *fmn_sprite_new(
  const struct fmn_sprtype *type,
  const struct fmn_sprdef *def,
  int16_t xmm,int16_t ymm,
  uint8_t q1,uint8_t q2,uint8_t q3
);

// Only fmn_sprite_new() should call this.
struct fmn_sprite *fmn_sprite_alloc();

/* Sprite type.
 ******************************************************************/
 
struct fmn_sprtype {
  const char *name;
  
  // Optional.
  void (*del)(struct fmn_sprite *sprite);
  int8_t (*init)(struct fmn_sprite *sprite,const struct fmn_sprdef *def);
  void (*update)(struct fmn_sprite *sprite);
  
  // Required.
  // Note that scroll is in mm -- not tiles, not pixels.
  void (*render)(
    struct fmn_image *dst,
    int16_t xscrollmm,int16_t yscrollmm,
    struct fmn_sprite *sprite
  );
  
  // Required. Return the sprite's physical bounds relative to (sprite->x,y) (ie can usually be constant).
  void (*hitbox)(int16_t *rx,int16_t *ry,int16_t *w,int16_t *h,struct fmn_sprite *sprite);
};

// Render with (sprite->image,tileid,xform), one tile centered in the sprite's full bounds.
void fmn_sprite_render_default(struct fmn_image *dst,int16_t xscroll,int16_t yscroll,struct fmn_sprite *sprite);

// Two sensible defaults for hitbox.
void fmn_sprite_hitbox_none(int16_t *rx,int16_t *ry,int16_t *w,int16_t *h,struct fmn_sprite *sprite);
void fmn_sprite_hitbox_all(int16_t *rx,int16_t *ry,int16_t *w,int16_t *h,struct fmn_sprite *sprite);

extern const struct fmn_sprtype fmn_sprtype_dummy;
extern const struct fmn_sprtype fmn_sprtype_heroproxy;
extern const struct fmn_sprtype fmn_sprtype_bat;

/* Sprite resource.
 *****************************************************************/
 
struct fmn_sprdef {
  const struct fmn_sprtype *type;
  const struct fmn_image *image;
  uint8_t tileid;
  uint8_t xform;
  //TODO properties eg "solid", "pushable", "hazard"
};

#endif
