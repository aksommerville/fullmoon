/* fmn_title.h
 * Initial UI mode, where we show the title splash and wait for "new" or "password".
 */
 
#ifndef FMN_TITLE_H
#define FMN_TITLE_H

void fmn_title_begin();
void fmn_title_end();
void fmn_title_input(uint16_t input,uint16_t prev);
void fmn_title_update();
void fmn_title_render(struct fmn_image *fb);

#endif
