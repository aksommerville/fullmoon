#ifndef FULLMOON_H
#define FULLMOON_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
  extern "C" {
#endif

struct fmn_image;
struct fmn_sprtype;
struct fmn_sprdef;
struct fmn_sprite;
struct fmn_sprgrp;
struct fmn_map;

/***************************************************************
 * Game must implement.
 */
 
void setup();
void loop();

/***************************************************************
 * Platform must implement (genioc,thumby,macos)
 */
 
void fmn_platform_init();
void fmn_platform_update();
void fmn_platform_send_framebuffer(const void *fb);
uint16_t fmn_platform_read_input();

/* Populate (fb->v) and (fb->stride) with the platform's framebuffer.
 * You must call this if FMN_PLATFORM_FRAMEBUFFER and otherwise must not.
 * On the Tiny and Pico, we output video without scaling but with margins.
 * You're not expected to worry about that, just use (fb).
 * Other platforms, you must allocate (fb).
 */
void fmn_platform_init_framebuffer(struct fmn_image *fb);

/***************************************************************
 * Shared symbols and services.
 */

/* The low 6 buttons are all we have on Thumby.
 * Could add others for features only available on higher platforms, eg fullscreen toggle, quit...
 */
#define FMN_BUTTON_LEFT    0x0001
#define FMN_BUTTON_RIGHT   0x0002
#define FMN_BUTTON_UP      0x0004
#define FMN_BUTTON_DOWN    0x0008
#define FMN_BUTTON_A       0x0010
#define FMN_BUTTON_B       0x0020

#include "fmn_image.h"
#include "fmn_framebuffer.h"

#define FMN_UIMODE_TITLE    1
#define FMN_UIMODE_PLAY     2
#define FMN_UIMODE_PAUSE    3
#define FMN_UIMODE_PASSWORD 4
#define FMN_UIMODE_GAMEOVER 5
void fmn_set_uimode(uint8_t mode);

/* Business.
 ***********************************************************************/

void fmn_game_reset();

/* (pw) is the decoded "internal use" password (the only kind that units outside the password manager see).
 */
void fmn_game_reset_with_password(uint32_t pw);
uint32_t fmn_game_generate_password();

uint16_t fmn_game_get_state();
void fmn_game_set_state(uint16_t mask,uint16_t value);

// "password" = "state", for decoded passwords.
#define FMN_STATE_BROOM          0x0001 /* items... */
#define FMN_STATE_FEATHER        0x0002
#define FMN_STATE_WAND           0x0004
#define FMN_STATE_UMBRELLA       0x0008
#define FMN_STATE_LOCATION_MASK  0x0070 /* starting map... */
#define FMN_STATE_LOCATION_SHIFT      4
#define FMN_STATE_CASTLE_OPEN    0x0080 /* gameplay flags... */
#define FMN_STATE_WOLF_DEAD      0x0100

#define FMN_PASSWORD_LENGTH 5
uint32_t fmn_password_encode(uint32_t pw);
uint32_t fmn_password_decode(uint32_t display);

// Convenience to spare me some typing...
static inline void fmn_blit_tile(
  struct fmn_image *dst,int16_t dstx,int16_t dsty,
  const struct fmn_image *src,
  uint8_t tileid,uint8_t xform
) {
  fmn_blit(dst,dstx,dsty,src,(tileid&0x0f)*FMN_TILESIZE,(tileid>>4)*FMN_TILESIZE,FMN_TILESIZE,FMN_TILESIZE,xform);
}

#define FMN_ACTION_NONE     0
#define FMN_ACTION_BROOM    1
#define FMN_ACTION_FEATHER  2
#define FMN_ACTION_WAND     3
#define FMN_ACTION_UMBRELLA 4

#define FMN_DIR_NW  0x80
#define FMN_DIR_N   0x40
#define FMN_DIR_NE  0x20
#define FMN_DIR_W   0x10
#define FMN_DIR_CTR 0x00
#define FMN_DIR_E   0x08
#define FMN_DIR_SW  0x04
#define FMN_DIR_S   0x02
#define FMN_DIR_SE  0x01

/* A spell is a sequence of FMN_DIR_(N,S,E,W).
 * Does the thing and returns nonzero, if it's a known spell.
 */
uint8_t fmn_game_cast_spell(const uint8_t *src,uint8_t srcc);

/* Maps.
 * In general, the game outside fmn_map.c doesn't need to know this definition.
 ****************************************************/

struct fmn_map {
  uint8_t *v;
  uint8_t w,h; // Multiples of (FMN_SCREENW_TILES,FMN_SCREENH_TILES)
  uint8_t region; // 0..7, which map would we start at if we die at this one?
  
  const struct fmn_image *tilesheet;
  const uint8_t *tileprops; // 256 bytes, one for each tile
  
  // POI must sort by (y,x) (sic not x,y; they sort in scan order).
  uint16_t poic;
  struct fmn_map_poi {
    uint8_t x,y;
    uint8_t q[4];
    void *qp;
  } *poiv;
};

// poi.q[0]
#define FMN_POI_START          0x00 /* () default start point */
#define FMN_POI_DOOR           0x01 /* (dstx,dsty,_,map) point door */
#define FMN_POI_SPRITE         0x02 /* (arg,arg,arg,sprdef) npc spawn point */
#define FMN_POI_TREADLE        0x03 /* (arg,arg,arg,function) point trigger */
#define FMN_POI_VISIBILITY     0x04 /* (arg,arg,arg,function) screen trigger */
#define FMN_POI_PROXIMITY      0x05 /* (arg,arg,arg,function) proximity updater */
#define FMN_POI_EDGE_DOOR      0x06 /* (offsetmsb,offsetlsb,_,map) */
#define FMN_POI_INTERIOR_DOOR  0x07 /* (dstx,dsty) */
// When changing this list, please update src/tool/mapcvt/mapcvt_convert.c:mapcvt_eval_poi_q0() and src/editor/www/js/map/FullmoonMap.js:POI_NAMES.

// tileprops
#define FMN_TILE_SOLID      0x01
#define FMN_TILE_HOLE       0x02

#ifdef __cplusplus
  }
#endif

#endif
