#include "fullmoon.h"
#include <string.h>

static uint8_t fb_storage[(FMN_FBW*FMN_FBH)>>3]={0};
static struct fmn_image fb={
  .v=fb_storage,
  .w=FMN_FBW,
  .h=FMN_FBH,
  .stride=FMN_FBW,
  .fmt=FMN_FBFMT,
  .writeable=1,
};
static uint16_t pvinput=0;

extern const struct fmn_image titlesplash;
extern const struct fmn_image bgtiles;
extern const struct fmn_image mainsprites;

void setup() {
  fmn_platform_init();
}

int16_t witchx=10;
int16_t witchy=10;

void loop() {
  fmn_platform_update();
  
  uint16_t input=fmn_platform_read_input();
  if (input!=pvinput) {
    /*impulse movement*
    if ((input&FMN_BUTTON_LEFT)&&!(pvinput&FMN_BUTTON_LEFT)) witchx--;
    if ((input&FMN_BUTTON_RIGHT)&&!(pvinput&FMN_BUTTON_RIGHT)) witchx++;
    if ((input&FMN_BUTTON_UP)&&!(pvinput&FMN_BUTTON_UP)) witchy--;
    if ((input&FMN_BUTTON_DOWN)&&!(pvinput&FMN_BUTTON_DOWN)) witchy++;
    /**/
    pvinput=input;
  }
  
  switch (input&(FMN_BUTTON_LEFT|FMN_BUTTON_RIGHT)) {
    case FMN_BUTTON_LEFT: witchx--; break;
    case FMN_BUTTON_RIGHT: witchx++; break;
  }
  switch (input&(FMN_BUTTON_UP|FMN_BUTTON_DOWN)) {
    case FMN_BUTTON_UP: witchy--; break;
    case FMN_BUTTON_DOWN: witchy++; break;
  }
  
  if (input&FMN_BUTTON_A) {
    memcpy(fb.v,titlesplash.v,sizeof(fb_storage));
  } else {
    uint8_t tileid=0x00;
    int16_t dsty=0;
    for (;dsty<fb.h;dsty+=8) {
      int16_t dstx=0;
      for (;dstx<fb.w;dstx+=8,tileid++) {
        fmn_blit(&fb,dstx,dsty,&bgtiles,(tileid&15)*8,(tileid>>4)*8,8,8);
      }
    }
  }
  
  const int16_t mmperpixel=1;
  fmn_blit(&fb,(witchx/mmperpixel),(witchy/mmperpixel)+6,&mainsprites,0,16,8,8);
  fmn_blit(&fb,(witchx/mmperpixel),(witchy/mmperpixel),&mainsprites,0,0,8,8);
  
  fmn_platform_send_framebuffer(fb.v);
}
