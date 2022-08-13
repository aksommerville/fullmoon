/* fmn_gameover.h
 * Splash shown after the hero dies.
 * Also use for our "The End" victory splash.
 */
 
#ifndef FMN_GAMEOVER_H
#define FMN_GAMEOVER_H

void fmn_gameover_begin();
void fmn_gameover_end();
void fmn_gameover_input(uint16_t input,uint16_t pvinput);
void fmn_gameover_update();
void fmn_gameover_render(struct fmn_image *fb);

// Resets to DEAD at fmn_gameover_begin().
void fmn_gameover_set_disposition(uint8_t disposition);
#define FMN_GAMEOVER_DISPOSITION_DEAD 0
#define FMN_GAMEOVER_DISPOSITION_VICTORY 1

#endif
