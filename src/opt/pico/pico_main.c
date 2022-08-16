#include "akpico.h"
#include "game/fullmoon.h"
#include "pico/stdlib.h"

#define FRAME_RATE 60 /* hz */
#define FRAME_DELAY (1000000/FRAME_RATE) /* us */
#define SLEEP_LIMIT 100000 /* us */

//XXX TEMP while i figure out cc flags for tiny (must force FMN_USE_minisyni true for everyone)
const uint8_t song_sevencircles[]={0};
const uint8_t song_fullmoon[]={0};
const uint16_t song_sevencircles_length=sizeof(song_sevencircles);
const uint16_t song_fullmoon_length=sizeof(song_fullmoon);
int8_t minisyni_play_song() { return 0; }

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
