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

/* Shift to a neighbor screen.
 * (dx,dy) should be one of ((-1,0),(1,0),(0,-1),(0,1)) but we'll try to do whatever you ask for.
 * (0,0) will always "fail", tho arguably it does what you ask for.
 * This may shift to another map, if edge doors are in play. TODO how to notify, eg for rebuilding sprite list?
 * Returns nonzero on success, otherwise the current view remains.
 */
uint8_t fmn_map_navigate(int8_t dx,int8_t dy);

/* Load a new map (reloads, even if already loaded).
 * 'default' puts focus at the map's declared init position.
 * 'position' lets you stipulate focus in global tiles.
 */
uint8_t fmn_map_load_default(const struct fmn_map *map);
uint8_t fmn_map_load_position(const struct fmn_map *map,uint8_t x,uint8_t y);

void fmn_map_get_init_position(uint8_t *x,uint8_t *y);
void fmn_map_get_scroll(uint8_t *x,uint8_t *y);
void fmn_map_get_size(uint8_t *w,uint8_t *h);
void fmn_map_get_size_mm(int16_t *wmm,int16_t *hmm);

/* Check if a given box collides with any static geometry.
 * If so: Return nonzero, and fill (adjx,adjy) with displacement to the nearest legal position, or (0,0) if we can't find one.
 * Or no collision: Return zero.
 * We're not exhaustive about this. If the box collides with two solid cells, we only correct against one of them.
 * I'm thinking that's OK because you'll find the second one next frame.
 * All coordinates in global mm.
 * (collmask) is a combination of FMN_TILE_*, which ones count as a collision.
 */
uint8_t fmn_map_check_collision(int16_t *adjx,int16_t *adjy,int16_t x,int16_t y,int16_t w,int16_t h,uint8_t collmask);

#endif
