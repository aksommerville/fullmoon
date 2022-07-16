#include "thumc.h"
#include "game/fullmoon.h"
#include "pico/stdlib.h"

/* Main.
 */

void main() {
  setup();
  while (1) {
    //TODO better timing
    busy_wait_us_32(16000);
    loop();
  }
}

/* Platform hooks.
 */
 
void fmn_platform_init() {
  thumby_begin();
}
 
void fmn_platform_update() {
}
 
void fmn_platform_send_framebuffer(const void *fb) {
  thumby_send_framebuffer(fb,(72*40)>>3);
}

uint16_t fmn_platform_read_input() {
  return thumby_get_buttons();
}
