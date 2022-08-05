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

#define FMN_SPRITE_FLAG_SOLID     0x0001 /* hero can't walk thru */
#define FMN_SPRITE_FLAG_DEATHROW  0x0002

#define FMN_FOR_EACH_SPRITE_FLAG \
  _(SOLID)
 
struct fmn_sprite {
  const struct fmn_sprtype *type;
  int16_t x,y,w,h; // mm relative to map. Must contain both visual and physical bounds.
  int8_t layer; // for render sorting
  uint16_t flags;
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

/* This will clean up the sprite and remove it from the global list immediately.
 */
void fmn_sprite_del(struct fmn_sprite *sprite);

/* Prefer over fmn_sprite_del: Marks the sprite for deletion at the end of the update cycle.
 */
void fmn_sprite_del_later(struct fmn_sprite *sprite);

// Only the governor should call this.
void fmn_sprites_execute_deathrow();

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

/* Check grid and other sprites, is this sprite colliding with anything?
 * If (resolve), we will move sprites to eliminate the collision and fire callbacks as needed.
 * Otherwise, we only populate (adjx,adjy) with the esapement and return true or false.
 * If we return nonzero but set (adj) zero, it means the sprite is in an unescapable collision.
 */
uint8_t fmn_sprite_collide(
  int16_t *adjx,int16_t *adjy,
  struct fmn_sprite *sprite,
  uint8_t cellprops,
  uint8_t spriteflags,
  uint8_t resolve
);

/* "Speculative collision detection."
 * How far in mm can this sprite move in each of the cardinal directions?
 * I've deliberately chosen uint8_t as the result type, to make it clear we're not looking very far.
 * Optionally, you can refine this result to clamp to the screenful the sprite's center falls on.
 */
void fmn_sprite_measure_cardinal_freedom(
  uint8_t *lrtb,
  const struct fmn_sprite *sprite,
  uint8_t cellprops,
  uint8_t spriteflags
);
void fmn_sprite_limit_freedom_to_screen(
  const struct fmn_sprite *sprite,
  uint8_t *lrtb
);

/* Sprite type.
 ******************************************************************/
 
struct fmn_sprtype {
  const char *name;
  
  // Optional.
  void (*del)(struct fmn_sprite *sprite);
  int8_t (*init)(struct fmn_sprite *sprite,const struct fmn_sprdef *def);
  void (*update)(struct fmn_sprite *sprite);
  void (*push)(struct fmn_sprite *sprite,int8_t dx,int8_t dy); // if implemented, you get notified when the hero pushes you
  uint8_t (*featherspell)(struct fmn_sprite *sprite,const uint8_t *v,uint8_t c); // nonzero if accepted, ie blank caller's state. Check the tail.
  
  // Required.
  // Note that scroll is in mm -- not tiles, not pixels.
  void (*render)(
    struct fmn_image *dst,
    int16_t xscrollmm,int16_t yscrollmm,
    struct fmn_sprite *sprite
  );
};

// Render with (sprite->image,tileid,xform), one tile centered in the sprite's full bounds.
void fmn_sprite_render_default(struct fmn_image *dst,int16_t xscroll,int16_t yscroll,struct fmn_sprite *sprite);

extern const struct fmn_sprtype fmn_sprtype_dummy;
extern const struct fmn_sprtype fmn_sprtype_hero;
extern const struct fmn_sprtype fmn_sprtype_bat;
extern const struct fmn_sprtype fmn_sprtype_treasure;
extern const struct fmn_sprtype fmn_sprtype_hazard;
extern const struct fmn_sprtype fmn_sprtype_pushbox;
extern const struct fmn_sprtype fmn_sprtype_treadle;
extern const struct fmn_sprtype fmn_sprtype_blockade;
extern const struct fmn_sprtype fmn_sprtype_stompbox;
extern const struct fmn_sprtype fmn_sprtype_firewall;
extern const struct fmn_sprtype fmn_sprtype_raccoon;
extern const struct fmn_sprtype fmn_sprtype_missile;

/* Sprite resource.
 *****************************************************************/
 
struct fmn_sprdef {
  const struct fmn_sprtype *type;
  const struct fmn_image *image;
  uint8_t tileid;
  uint8_t xform;
  uint16_t flags;
  int8_t layer;
};

#endif
