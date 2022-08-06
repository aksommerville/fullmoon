/* fmn_play.h
 * Main UI mode, moving the hero around and doing stuff.
 */
 
#ifndef FMN_PLAY_H
#define FMN_PLAY_H

extern uint32_t fmn_play_frame_count; // since reset

void fmn_play_begin();
void fmn_play_end();
void fmn_play_input(uint16_t input,uint16_t prev);
void fmn_play_update();
void fmn_play_render(struct fmn_image *fb);

void fmn_bgbits_dirty();

/* Signal that the hero is dead and we should wait a tasteful interval, then switch to GAMEOVER.
 * If nonzero (x,y) is the position in mm where the hero was; we may put some fireworks there.
 */
void fmn_game_end(int16_t x,int16_t y);

#endif
