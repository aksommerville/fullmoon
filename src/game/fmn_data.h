#ifndef FMN_DATA_H
#define FMN_DATA_H

extern const struct fmn_image titlesplash;
extern const struct fmn_image bgtiles;
extern const struct fmn_image mainsprites;
extern const struct fmn_image uibits;
extern const struct fmn_image fanfare;
extern const struct fmn_image castlesprites;

extern const struct fmn_map cheatertrap; // TODO Remove, instead validate passwords refer to a real map and reject.
extern const struct fmn_map home;
extern const struct fmn_map forest;
extern const struct fmn_map castle_1;
extern const struct fmn_map mountains;
extern const struct fmn_map desert_1;
extern const struct fmn_map swamp_1;

extern const struct fmn_map *map_region_heads[8];

extern const uint8_t song_cobweb[];
extern const uint16_t song_cobweb_length;
extern const uint8_t song_baltic[];
extern const uint16_t song_baltic_length;
extern const uint8_t song_fullmoon[];
extern const uint16_t song_fullmoon_length;
extern const uint8_t song_infinite[];
extern const uint16_t song_infinite_length; // no, no, it only *feels* that long
extern const uint8_t song_sevencircles[];
extern const uint16_t song_sevencircles_length;

#endif
