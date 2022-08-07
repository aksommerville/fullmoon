#include "test/fmn_test.h"
#include "game/ui/fmn_password.c"
#include "game/fullmoon.h"
#include <stdlib.h>

void fmn_game_reset_with_state(uint16_t state) {}
int8_t fmn_map_validate_region(uint8_t region) { return 0; }
void fmn_set_uimode(uint8_t mode) {}
const struct fmn_image uibits={0};
void fmn_blit(struct fmn_image *dst,int16_t dstx,int16_t dsty,const struct fmn_image *src,int16_t srcx,int16_t srcy,int16_t w,int16_t h,uint8_t xform) {}

/* Exhaustive password encode.
 * There are 64k expressible states, of which only a fraction will be fully valid.
 */

static int test_password_for_all_states() {

  // The first 5 are expected: OK,LOCATION,RESERVED,SEQUENCE,BUSINESS.
  // We will never get LOCATION, because it's generated at fmn_map_validate_region, stubbed above.
  // resultv[5] is for any other result, which is an error.
  int resultv[6]={0};

  uint16_t state=0;
  while (1) {
    char password[FMN_PASSWORD_LENGTH];
    int8_t err=fmn_password_repr(password,state);
    if ((err>=0)&&(err<5)) {
    
      if (err==FMN_PASSWORD_OK) {
        uint16_t evalstate=0;
        int8_t evalerr=fmn_password_eval(&evalstate,password);
        FMN_ASSERT_INTS(evalerr,FMN_PASSWORD_OK,"0x%04x => '%.6s', re-evaluate failed",state,password)
        FMN_ASSERT_INTS(evalstate,state,"0x%04x => '%.6s' => 0x%04x",state,password,evalstate)
        // As business rules change, it's wise to return here sometimes and confirm we're not generating obscene passwords.
        // "FUQQQT" has come up before...
        // Any time before the first release, it's fine to shuffle fmn_password_{c,h,s}_pos to get different text.
        //fprintf(stderr,"0x%04x => %.6s\n",state,password);
      }
      
      resultv[err]++;
      
      // Generated passwords must only contain A..Z,1,3,4,6,7,9
      const char *p=password;
      int i=FMN_PASSWORD_LENGTH;
      for (;i-->0;p++) {
        if ((*p>='A')&&(*p<='Z')) continue;
        if (*p=='1') continue;
        if (*p=='3') continue;
        if (*p=='4') continue;
        if (*p=='6') continue;
        if (*p=='7') continue;
        if (*p=='9') continue;
        FMN_FAIL("Unexpected character 0x%02x in password: 0x%04x => '%.6s'",*p,state,password);
      }
      
    } else {
      fprintf(stderr,"0x%04x => !!! %d\n",state,err);
      resultv[5]++;
    }
    if (state==0xffff) break;
    state++;
  }
  
  FMN_ASSERT_INTS(resultv[5],0,"Unexpected result from fmn_password_repr -- enable logging here to see it.")
  
  FMN_ASSERT_INTS(resultv[1],0,"Expected no FMN_PASSWORD_LOCATION, as fmn_map_validate_region is stubbed for test purposes")
  FMN_ASSERT_INTS(resultv[4],0,"Expected no FMN_PASSWORD_BUSINESS. Did you add a rule? Can it be more specific?") // ok to change this assumption
  FMN_ASSERT_INTS_OP(resultv[0],<,0x1000,"Expected less than 1/16 of possible states to yield a valid password. It's OK to bump this number.")
  
  if (0) { // dump the histogram of results
    fprintf(stderr,"fmn_password_repr:\n");
    fprintf(stderr,"        OK: %5d\n",resultv[0]);
    fprintf(stderr,"  LOCATION: %5d\n",resultv[1]);
    fprintf(stderr,"  RESERVED: %5d\n",resultv[2]);
    fprintf(stderr,"  SEQUENCE: %5d\n",resultv[3]);
    fprintf(stderr,"  BUSINESS: %5d\n",resultv[4]);
  }
  
  return 0;
}

/* fmn_password_{c,h,s}_pos must collectively have 30 entries, containing the numbers 0..29 each exactly once.
 */
 
static int validate_bin_bits() {
  uint32_t bits=0;
  uint8_t i=0,c=0;
  for (i=0;i<sizeof(fmn_password_c_pos);i++,c++) bits|=1<<fmn_password_c_pos[i];
  for (i=0;i<sizeof(fmn_password_h_pos);i++,c++) bits|=1<<fmn_password_h_pos[i];
  for (i=0;i<sizeof(fmn_password_s_pos);i++,c++) bits|=1<<fmn_password_s_pos[i];
  FMN_ASSERT_INTS(c,30)
  FMN_ASSERT_INTS(bits,0x3fffffff)
  return 0;
}

/* There's a finite and rather small set of fully-valid passwords that we expect to arise in real use.
 * Enumerate those states here, and optionally log all of the encoded passwords so we can manually validate entropy.
 */
 
static int test_known_valid_passwords() {
  const int log=0;
  #define _(locationid,state,comment) { \
    char password[FMN_PASSWORD_LENGTH]; \
    int8_t err=fmn_password_repr(password,state|(locationid<<FMN_STATE_LOCATION_SHIFT)); \
    FMN_ASSERT_INTS(err,FMN_PASSWORD_OK,"loc=%d state=0x%04x comment=%s",locationid,state,comment) \
    if (log) fprintf(stderr,"%.6s %d 0x%04x %s\n",password,locationid,state,comment); \
  }
  
  _(1,0,"initial")
  _(2,0,"visit forest")
  _(2,FMN_STATE_FEATHER,"get feather")
  _(1,FMN_STATE_FEATHER,"after getting feather")
  //TODO finish the narrative...

  #undef _
  return 0;
}

/* Validate lexical scrubbing and error checking.
 */
 
static int test_password_normalization() {
  uint16_t state;
  // Only FMN_PASSWORD_ARGUMENT takes precedence over FMN_PASSWORD_ILLEGAL_CHAR.
  // So we don't care whether checksums or business rules pass, just look for ILLEGAL_CHAR.
  #define VALID(pw) FMN_ASSERT_INTS_OP(fmn_password_eval(&state,pw),!=,FMN_PASSWORD_ILLEGAL_CHAR,"'%s'",pw)
  #define INVALID(pw) FMN_ASSERT_INTS(fmn_password_eval(&state,pw),FMN_PASSWORD_ILLEGAL_CHAR,"'%s'",pw)
  
  VALID("ABCDEF")
  VALID("GHIJKL")
  VALID("MNOPQR")
  VALID("STUVWX")
  VALID("YZaaaa")
  VALID("abcdef")
  VALID("ghijkl")
  VALID("mnopqr")
  VALID("stuvwx")
  VALID("yzAAAA")
  VALID("134679")
  VALID("0258aa") // "0258" are legal; they turn into "OZSB"
  
  INVALID("aaaaa")
  INVALID("aaaaa ")
  INVALID("aaaaa!")
  INVALID("aaaaa|")
  INVALID("aaaaa?")
  INVALID("aaaaa.")
  INVALID("aaaaa,")
  VALID("aaaaaa")
  
  #undef VALID
  #undef INVALID
  return 0;
}

/* TOC.
 */

int main(int argc,char **argv) {
  FMN_UTEST(test_password_for_all_states)
  FMN_UTEST(validate_bin_bits)
  FMN_UTEST(test_known_valid_passwords)
  FMN_UTEST(test_password_normalization)
  return 0;
}
