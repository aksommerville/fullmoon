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

#endif
