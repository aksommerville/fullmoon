#ifndef FMN_DATA_H
#define FMN_DATA_H

extern const struct fmn_image titlesplash;
extern const struct fmn_image bgtiles;
extern const struct fmn_image mainsprites;
extern const struct fmn_image uibits;

extern const struct fmn_map outermap;//XXX
extern const struct fmn_map cave001;//XXX
extern const struct fmn_map cheatertrap; // TODO Remove, instead validate passwords refer to a real map and reject.
extern const struct fmn_map home;

extern const struct fmn_map *map_region_heads[8];

#endif
