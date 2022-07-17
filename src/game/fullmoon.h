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

#endif
