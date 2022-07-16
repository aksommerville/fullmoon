#include "game/fullmoon.h"
#include "pico/stdlib.h"

void main() {
  setup();
  while (1) {
    //TODO better timing
    busy_wait_us_32(16000);
    loop();
  }
}
