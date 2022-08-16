#include "akpico.h"
#include "game/fullmoon.h"
#include "pico/stdlib.h"

#define FRAME_RATE 60 /* hz */
#define FRAME_DELAY (1000000/FRAME_RATE) /* us */
#define SLEEP_LIMIT 100000 /* us */

/* Main.
 */

void main() {
  akpico_init();
  setup();
  while (1) {
    akpico_update();
    loop();
  }
}

/* Platform hooks.
 */
 
void fmn_platform_init() {
}
 
void fmn_platform_update() {
}
 
void fmn_platform_send_framebuffer(const void *fb) {
  akpico_send_framebuffer(fb);
}

uint16_t fmn_platform_read_input() {
  return akpico_get_buttons();
}
