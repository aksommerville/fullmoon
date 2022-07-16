#include "fullmoon.h"
#include <string.h>

static uint8_t fb[(FMN_FBW*FMN_FBH)>>3]={0};
static uint16_t pvinput=0;

extern const struct fmn_image titlesplash;

void setup() {
  fmn_platform_init();
}

void loop() {
  fmn_platform_update();
  
  uint16_t input=fmn_platform_read_input();
  if (input!=pvinput) {
    pvinput=input;
  }

  // gray out buffer.
  uint8_t gray=0x55;
  uint8_t *v=fb;
  uint16_t i=sizeof(fb);
  for (;i-->0;v++,gray^=0xff) *v=gray;
  
  memcpy(fb,titlesplash.v,sizeof(fb));
  
  fmn_platform_send_framebuffer(fb);
}
