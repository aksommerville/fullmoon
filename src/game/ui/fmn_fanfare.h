#ifndef FMN_FANFARE_H
#define FMN_FANFARE_H

void fmn_fanfare_begin();
void fmn_fanfare_end();
void fmn_fanfare_input(uint16_t input,uint16_t pvinput);
void fmn_fanfare_update();
void fmn_fanfare_render(struct fmn_image *fb);

void fmn_fanfare_set_action(uint8_t action);

#endif
