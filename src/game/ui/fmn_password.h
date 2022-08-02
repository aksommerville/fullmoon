/* fmn_password.h
 * UI mode for password entry, from title splash.
 */
 
#ifndef FMN_PASSWORD_H
#define FMN_PASSWORD_H

void fmn_password_begin();
void fmn_password_end();
void fmn_password_input(uint16_t input,uint16_t prev);
void fmn_password_update();
void fmn_password_render(struct fmn_image *fb);

#endif
