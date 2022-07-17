/* fmn_play.h
 * Main UI mode, moving the hero around and doing stuff.
 */
 
#ifndef FMN_PLAY_H
#define FMN_PLAY_H

void fmn_play_begin();
void fmn_play_end();
void fmn_play_input(uint16_t input,uint16_t prev);
void fmn_play_update();
void fmn_play_render(struct fmn_image *fb);

void fmn_bgbits_dirty();

// Top-level navigation to screen neighbors. Nonzero if the view changed.
uint8_t fmn_game_navigate(int8_t dx,int8_t dy);

// Scroll if needed, to focus the given point.
uint8_t fmn_game_focus_mm(int16_t xmm,int16_t ymm);

/* A spell is a sequence of FMN_DIR_(N,S,E,W).
 * Does the thing and returns nonzero, if it's a known spell.
 */
uint8_t fmn_game_cast_spell(const uint8_t *src,uint8_t srcc);

#endif
