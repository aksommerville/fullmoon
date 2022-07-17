#ifndef FULLMOON_H
#define FULLMOON_H

#include <stdint.h>
#include <stdio.h>

/***************************************************************
 * Game must implement.
 */
 
void setup();
void loop();

/***************************************************************
 * Platform must implement (genioc,thumby)
 */
 
void fmn_platform_init();
void fmn_platform_update();
void fmn_platform_send_framebuffer(const void *fb);
uint16_t fmn_platform_read_input();

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

#define FMN_IMGFMT_thumby     1 /* The Thumby framebuffer. y1 where each byte is 8 rows (not columns!) */
#define FMN_IMGFMT_ya11       2 /* Big-endian 2-bit luma+alpha, for sprites. */
#define FMN_IMGFMT_bgr565be   3
#define FMN_IMGFMT_rgba8888   4
#define FMN_IMGFMT_y1         5 /* Ordinary big-endian y1. */
#define FMN_IMGFMT_y8         6

// Not negotiable.
#define FMN_FBW 72
#define FMN_FBH 40
#define FMN_FBFMT FMN_IMGFMT_thumby

struct fmn_image {
  uint8_t *v;
  // NB size of (v) is not necessarily (h*stride); thumby format is different.
  int16_t w,h,stride;
  uint8_t fmt;
  uint8_t writeable;
};

void fmn_blit(
  struct fmn_image *dst,int16_t dstx,int16_t dsty,
  const struct fmn_image *src,int16_t srcx,int16_t srcy,
  int16_t w,int16_t h
);

#define FMN_UIMODE_TITLE    1
#define FMN_UIMODE_PLAY     2
#define FMN_UIMODE_PAUSE    3
#define FMN_UIMODE_PASSWORD 4
void fmn_set_uimode(uint8_t mode);

void fmn_game_reset();

/* (pw) is the decoded "internal use" password (the only kind that units outside the password manager see).
 */
void fmn_game_reset_with_password(uint32_t pw);
uint32_t fmn_game_generate_password();

#define FMN_PASSWORD_LENGTH 5
uint32_t fmn_password_encode(uint32_t pw);
uint32_t fmn_password_decode(uint32_t display);

// (FMN_SCREENW_TILES*FMN_TILESIZE==FMN_FBW) and (FMN_SCREENH_TILES*FMN_TILESIZE==FMN_FBH)
// (FMN_MM_PER_PIXEL*FMN_TILESIZE==FMN_MM_PER_TILE)
// (FMN_MM_PER_TILE*255<0x8000)
#define FMN_TILESIZE 8
#define FMN_SCREENW_TILES 9
#define FMN_SCREENH_TILES 5
#define FMN_MM_PER_TILE 64 /* <128, and power of two probably a good idea */
#define FMN_MM_PER_PIXEL 8
#define FMN_SCREENW_MM (FMN_SCREENW_TILES*FMN_MM_PER_TILE)
#define FMN_SCREENH_MM (FMN_SCREENH_TILES*FMN_MM_PER_TILE)

struct fmn_map {
  uint8_t *v;
  uint8_t w,h; // Multiples of (FMN_SCREENW_TILES,FMN_SCREENH_TILES)
  uint8_t initx,inity; // Hero's default start position.
};

// Convenience to spare me some typing...
static inline void fmn_blit_tile(
  struct fmn_image *dst,int16_t dstx,int16_t dsty,
  const struct fmn_image *src,
  uint8_t tileid
) {
  fmn_blit(dst,dstx,dsty,src,(tileid&0x0f)*FMN_TILESIZE,(tileid>>4)*FMN_TILESIZE,FMN_TILESIZE,FMN_TILESIZE);
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

#define FMN_SPELL_LENGTH_LIMIT 10

#endif
