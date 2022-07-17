/* fmn_pause.h
 * In-game pause menu, where you select a weapon.
 */
 
#ifndef FMN_PAUSE_H
#define FMN_PAUSE_H

void fmn_pause_begin();
void fmn_pause_end();
void fmn_pause_input(uint16_t input,uint16_t prev);
void fmn_pause_update();
void fmn_pause_render(struct fmn_image *fb);
uint8_t fmn_pause_get_action();

#endif
