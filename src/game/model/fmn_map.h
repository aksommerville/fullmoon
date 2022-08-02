/* fmn_map.h
 * Manages visuals and static geometry for the background layer.
 */
 
#ifndef FMN_MAP_H
#define FMN_MAP_H

/* Get the portion of the map currently in view.
 * Beware that it might have a stride much larger than FMN_SCREENW_TILES.
 * This never fails. If a map is not loaded, we return a valid dummy with all zeroes.
 */
uint8_t *fmn_map_get_view(uint8_t *stride);

// Return to the game's starting point.
void fmn_map_reset();
void fmn_map_reset_region(uint8_t region);

/* Check hero position.
 * May change the current map or current scroll.
 * If so, we trigger proximity and visibility POI as warranted.
 * And we may rebuild the sprite set.
 */
void fmn_map_update(int16_t herox,int16_t heroy);

// Store this map and hero position (in tiles) to be committed at the next update.
void fmn_map_load_soon(struct fmn_map *map,uint8_t x,uint8_t y);

void fmn_map_get_init_position(uint8_t *x,uint8_t *y);
void fmn_map_get_scroll(uint8_t *x,uint8_t *y);
void fmn_map_get_scroll_mm(int16_t *xmm,int16_t *ymm);
void fmn_map_get_size(uint8_t *w,uint8_t *h);
void fmn_map_get_size_mm(int16_t *wmm,int16_t *hmm);

/* Update DOOR and TREADLE POIs.
 */
uint8_t fmn_map_enter_cell(uint8_t x,uint8_t y);
void fmn_map_exit_cell(uint8_t x,uint8_t y);

void fmn_map_call_visibility_pois(uint8_t state);
void fmn_map_add_proximity_pois();
struct fmn_map *fmn_map_find_edge_door_left(const struct fmn_map *from,int16_t vytiles,int16_t *dxmm,int16_t *dymm);
struct fmn_map *fmn_map_find_edge_door_right(const struct fmn_map *from,int16_t vytiles,int16_t *dxmm,int16_t *dymm);
struct fmn_map *fmn_map_find_edge_door_up(const struct fmn_map *from,int16_t vxtiles,int16_t *dxmm,int16_t *dymm);
struct fmn_map *fmn_map_find_edge_door_down(const struct fmn_map *from,int16_t vxtiles,int16_t *dxmm,int16_t *dymm);

int8_t fmn_map_for_each_poi(
  uint8_t x,uint8_t y,uint8_t w,uint8_t h,
  int8_t (*cb)(const struct fmn_map_poi *poi,void *userdata),
  void *userdata
);

uint8_t fmn_map_get_region();
const struct fmn_map *fmn_map_get();

#endif
