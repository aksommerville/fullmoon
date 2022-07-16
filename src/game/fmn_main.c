#include "fullmoon.h"
#include <string.h>

#if FMN_USE_thumby
  #include "opt/thumby/thumc.h"
#endif

static uint8_t fb[(72*40)>>3]={0};

void setup() {
  #if FMN_USE_thumby
    thumby_begin();
  #endif
}

void loop() {

  // gray out buffer.
  uint8_t gray=0x55;
  uint8_t *v=fb;
  uint16_t i=sizeof(fb);
  for (;i-->0;v++,gray^=0xff) *v=gray;
  
  #if FMN_USE_thumby
    thumby_send_framebuffer(fb,sizeof(fb));
  #endif
}
