#include "test/fmn_test.h"
#include "game/ui/fmn_password.c"
#include "game/fullmoon.h"
#include <stdlib.h>

void fmn_game_reset_with_password(uint32_t pw) {}
void fmn_set_uimode(uint8_t mode) {}
const struct fmn_image uibits={0};
void fmn_blit(struct fmn_image *dst,int16_t dstx,int16_t dsty,const struct fmn_image *src,int16_t srcx,int16_t srcy,int16_t w,int16_t h,uint8_t xform) {}

/* Exhaustive password encode/decode.
 */

static int test_passwords_exhaustively() {
  
  // There are 32k valid passwords (ie every integer that fits in 15 bits).
  // Confirm that each of them produces a unique 25-bit password.
  uint32_t *displayv=malloc(sizeof(uint32_t)*0x8000);
  FMN_ASSERT(displayv)
  int displayc=0;
  uint32_t pw=0;
  for (;pw<0x8000;pw++) {
    uint32_t display=fmn_password_encode(pw);
    FMN_ASSERT_INTS_OP(display,!=,0xffffffff,"fmn_password_encode(0x%04x)",pw)
    uint32_t repw=fmn_password_decode(display);
    FMN_ASSERT_INTS(pw,repw,"password 0x%04x => 0x%08x => 0x%04x",pw,display,repw)
    int i=displayc; while (i-->0) FMN_ASSERT_INTS_OP(display,!=,displayv[i],"Duplicate password 0x%08x for 0x%04x and 0x%04x",display,pw,i)
  }
  free(displayv);
  
  // There are 32M valid display passwords.
  // Decode each and confirm that exactly 32k come out legal.
  uint32_t display=0,validc=0;
  for (;display<0x02000000;display++) {
    uint32_t pw=fmn_password_decode(display);
    if (pw!=0xffffffff) validc++;
  }
  FMN_ASSERT_INTS(validc,0x8000)
  
  return 0;
}

/* Generate a password. Not a test, just a helper for manual testing.
 */
 
static int generate_password() {

  // CCOHL: Straight zeroes.
  // BVLUH: All items, location 0.
  // BCOBH: Broom+umbrella, location 0.

  uint32_t state=
    FMN_STATE_BROOM|
    //FMN_STATE_FEATHER|
    //FMN_STATE_WAND|
    FMN_STATE_UMBRELLA|
    (7<<FMN_STATE_LOCATION_SHIFT)|
    //FMN_STATE_CASTLE_OPEN|
    //FMN_STATE_WOLF_DEAD|
  0;
  
  char password[5];
  uint32_t display=fmn_password_encode(state);
  uint8_t i=0; for (;i<5;i++) password[i]=(display>>(i*5))&0x1f;
  
  const char *alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZ134679";
  for (i=0;i<5;i++) password[i]=alphabet[password[i]];
  fprintf(stderr,"Password for state 0x%04x: %.5s\n",state,password);
  return 0;
}

/* TOC.
 */

int main(int argc,char **argv) {
  XXX_FMN_UTEST(test_passwords_exhaustively) // can take like 10 s to run
  FMN_UTEST(generate_password)
  return 0;
}
