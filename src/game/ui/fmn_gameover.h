/* fmn_gameover.h
 * Splash shown after the hero dies.
 */
 
#ifndef FMN_GAMEOVER_H
#define FMN_GAMEOVER_H

void fmn_gameover_begin();
void fmn_gameover_end();
void fmn_gameover_input(uint16_t input,uint16_t pvinput);
void fmn_gameover_update();
void fmn_gameover_render(struct fmn_image *fb);

#endif
