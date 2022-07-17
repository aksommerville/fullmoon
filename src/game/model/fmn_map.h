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
 * Returns nonzero on success, otherwise the current view remains.
 */
uint8_t fmn_map_navigate(int8_t dx,int8_t dy);

/* Load a new map (reloads, even if already loaded).
 * 'default' puts focus at the map's declared init position.
 * 'position' lets you stipulate focus in global tiles.
 */
uint8_t fmn_map_load_default(const struct fmn_map *map);
uint8_t fmn_map_load_position(const struct fmn_map *map,uint8_t x,uint8_t y);

#endif
