#include "thumc.h"
#include "game/fullmoon.h"
#include "pico/stdlib.h"

#define FRAME_RATE 60 /* hz */
#define FRAME_DELAY (1000000/FRAME_RATE) /* us */
#define SLEEP_LIMIT 100000 /* us */

/* Main.
 */

void main() {
  setup();
  uint64_t nexttime=time_us_64();
  while (1) {
  
    uint64_t now=time_us_64();
    while (1) {
      int64_t sleeptime=nexttime-now;
      if (sleeptime<=0) {
        nexttime+=FRAME_DELAY;
        if (nexttime<=now) { // panic
          nexttime=now+FRAME_DELAY;
        }
        break;
      }
      if (sleeptime>SLEEP_LIMIT) { // panic
        nexttime=now+FRAME_DELAY;
        break;
      }
      busy_wait_us_32(sleeptime);
      now=time_us_64();
    }

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
