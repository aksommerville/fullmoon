#include "fullmoon.h"
#include "fmn_password.h"
#include "fmn_data.h"
#include <string.h>

/* Globals.
 */
 
static uint8_t fbdirty=0;
static uint8_t cursorp=0;
static uint8_t cursorframe=0;
static uint8_t cursortime=0;
static uint8_t password[FMN_PASSWORD_LENGTH]={0}; // 0=unset, 1..32=chars

/* The password system.
 * There are "decoded" aka "display" passwords which are just the displayed digits, 5 bits each.
 * Then there's "encoded" aka "pw", which is a valid state of the game.
 * pw 0xffffffff is the error signal.
 */
 
static const uint8_t pwglyph0[32]={2,27,29,26,18,7,17,0,16,1,19,28,4,10,8,24,31,23,30,11,12,15,25,3,9,13,5,21,20,22,6,14};
static const uint8_t pwglyph1[32]={2,9,15,30,28,1,14,3,27,21,6,11,10,31,12,22,25,13,16,18,20,19,0,24,17,26,29,8,5,4,23,7};
static const uint8_t pwglyph2[32]={14,30,10,16,13,25,29,1,26,24,20,21,8,3,12,22,31,17,9,5,19,18,2,7,23,0,28,11,27,15,4,6};
static const uint8_t pwglyph3[32]={11,31,0,5,26,23,30,8,24,1,16,18,12,13,15,14,9,4,21,25,29,17,2,28,7,22,6,20,3,19,10,27};
static const uint8_t pwglyph4[32]={7,31,9,2,21,11,12,24,6,4,14,29,22,13,10,17,5,8,1,0,25,26,15,19,30,27,20,16,28,18,3,23};

static uint8_t findpw(const uint8_t *v,uint8_t display) {
  uint8_t i=0; for (;i<32;i++) {
    if (v[i]==display) return i;
  }
  return 0xff;
}
 
uint32_t fmn_password_decode(uint32_t display) {

  // Split, and undo the display obfuscation.
  uint8_t v[5];
  if ((v[0]=findpw(pwglyph0,(display>> 0)&0x1f))>31) return 0xffffffff;
  if ((v[1]=findpw(pwglyph1,(display>> 5)&0x1f))>31) return 0xffffffff;
  if ((v[2]=findpw(pwglyph2,(display>>10)&0x1f))>31) return 0xffffffff;
  if ((v[3]=findpw(pwglyph3,(display>>15)&0x1f))>31) return 0xffffffff;
  if ((v[4]=findpw(pwglyph4,(display>>20)&0x1f))>31) return 0xffffffff;
  
  // Reassemble interleaved.
  uint32_t pw=0,shift=0;
  uint8_t i=0; for (;i<5;i++) {
    uint8_t j=0; for (;j<5;j++,shift++) {
      pw|=(v[j]&(1<<i))?(1<<shift):0;
    }
  }
  
  // Now the high 10 bits are a hash of the low 15.
  uint32_t expect=pw>>15;
  uint32_t actual=(pw&0x3ff)^((pw&0x7c00)>>5)^((pw&0x7c00)>>10);
  if (expect!=actual) return 0xffffffff;
  
  // It's good! Strip off the hash and return it.
  return pw&0x7fff;
}

uint32_t fmn_password_encode(uint32_t pw) {

  // (pw) must use only the low 15 bits.
  if (pw&~0x7fff) return 0xffffffff;
  
  // Calculate and append hash.
  uint32_t hash=(pw&0x3ff)^((pw&0x7c00)>>5)^((pw&0x7c00)>>10);
  pw|=hash<<15;
  
  // Deinterleave into five five-bit buckets.
  uint8_t v[5]={0};
  uint32_t mask=1;
  uint8_t i=0; for (;i<5;i++) {
    uint8_t j=0; for (;j<5;j++,mask<<=1) {
      v[j]|=(pw&mask)?(1<<i):0;
    }
  }

  // Apply display obfuscation.
  v[0]=pwglyph0[v[0]];
  v[1]=pwglyph1[v[1]];
  v[2]=pwglyph2[v[2]];
  v[3]=pwglyph3[v[3]];
  v[4]=pwglyph4[v[4]];
  
  // Combine those buckets and we're done.
  return v[0]|(v[1]<<5)|(v[2]<<10)|(v[3]<<15)|(v[4]<<20);
}

/* Begin.
 */
 
void fmn_password_begin() {
  fbdirty=1;
  memset(password,0,sizeof(password));
  cursorp=0;
}

/* End.
 */
 
void fmn_password_end() {
}

/* Submit password.
 */
 
static void fmn_password_submit() {
  uint32_t packed=0,shift=0;
  int i=0; for (;i<FMN_PASSWORD_LENGTH;i++,shift+=5) {
    if ((password[i]<1)||(password[i]>32)) return;
    packed|=(password[i]-1)<<shift;
  }
  uint32_t pw=fmn_password_decode(packed);
  if (pw!=0xffffffff) {
    fmn_game_reset_with_password(pw);
    fmn_set_uimode(FMN_UIMODE_PLAY);
  }
}

/* Adjust one character.
 */
 
static void modify_char(int8_t d) {
  if (d<0) {
    if (password[cursorp]<=1) password[cursorp]=32;
    else password[cursorp]--;
  } else {
    if (password[cursorp]==32) password[cursorp]=1;
    else password[cursorp]++;
  }
}

/* Input.
 */
 
void fmn_password_input(uint16_t input,uint16_t prev) {
  fbdirty=1;
  
  if ((input&FMN_BUTTON_LEFT)&&!(prev&FMN_BUTTON_LEFT)) {
    if (cursorp) cursorp--;
    else cursorp=FMN_PASSWORD_LENGTH-1;
  }
  
  if ((input&FMN_BUTTON_RIGHT)&&!(prev&FMN_BUTTON_RIGHT)) {
    cursorp++;
    if (cursorp>=FMN_PASSWORD_LENGTH) cursorp=0;
  }
  
  if ((input&FMN_BUTTON_UP)&&!(prev&FMN_BUTTON_UP)) {
    modify_char(1);
  }
  
  if ((input&FMN_BUTTON_DOWN)&&!(prev&FMN_BUTTON_DOWN)) {
    modify_char(-1);
  }
  
  if ((input&FMN_BUTTON_B)&&!(prev&FMN_BUTTON_B)) {
    fmn_set_uimode(FMN_UIMODE_TITLE);
  }
  
  if ((input&FMN_BUTTON_A)&&!(prev&FMN_BUTTON_A)) {
    fmn_password_submit();
  }
}

/* Update.
 */
 
void fmn_password_update() {
  if (cursortime) cursortime--;
  else {
    cursortime=7;
    cursorframe++;
    if (cursorframe>=6) cursorframe=0;
    fbdirty=1;
  }
}

/* Render.
 */
 
void fmn_password_render(struct fmn_image *fb) {
  if (!fbdirty) return;
  fbdirty=0;
  fmn_blit(fb,0,0,&uibits,0,24,72,40);
  int16_t dstx=5,i;
  for (i=0;i<FMN_PASSWORD_LENGTH;i++,dstx+=13) {
    if (password[i]<1) ;
    else if (password[i]<=12) fmn_blit(fb,dstx,14,&uibits,(password[i]-1)*10,64,10,10);
    else if (password[i]<=24) fmn_blit(fb,dstx,14,&uibits,(password[i]-13)*10,74,10,10);
    else if (password[i]<=32) fmn_blit(fb,dstx,14,&uibits,(password[i]-25)*10,84,10,10);
  }
  fmn_blit(fb,6+cursorp*13,9,&uibits,48+cursorframe*7,0,7,5);
  fmn_blit(fb,6+cursorp*13,24,&uibits,48+cursorframe*7,5,7,5);
}
