/* akpico.h
 * Stripped-down interface to PicoSystem, with C linkage.
 */
 
#ifndef AKPICO_H
#define AKPICO_H

#include <stdint.h>

void akpico_init();
void akpico_update();
uint16_t akpico_get_buttons();
void akpico_send_framebuffer(const void *src);

#endif
