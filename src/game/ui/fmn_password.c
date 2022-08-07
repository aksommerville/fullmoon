#include "game/fullmoon.h"
#include "game/ui/fmn_password.h"
#include "game/model/fmn_map.h"
#include "game/fmn_data.h"
#include <string.h>

/* Globals.
 */
 
static uint8_t fbdirty=0;
static uint8_t cursorp=0;
static uint8_t cursorframe=0;
static uint8_t cursortime=0;
static char entry[FMN_PASSWORD_LENGTH]="AAAAAA";

/* Password format.
 * There is a third password format "bin" which is only used here.
 * That is a close representation of the text password -- text is produced by reading 5 bits at a time from bin, little-endianly.
 * The top 2 bits of bin are not used.
 * Generating a state from a bin validates the checksum but not the business rules.
 *
 * state<=>bin is where the real action happens.
 * bin is 5 fields: xxCC CCCh hhhh hhhh ssss ssss ssss ssss
 *   'x' 2 bits always zero, always at the top.
 *   'C' 5 bit checksum, forces popcnt to 32.
 *   'h' 9 bit hash: ~((statehi<<1)^statelo) # ensures that there be at least 1 'on' bit, and any 1-bit change affects at least 2 bits.
 *   's' 16 bit plaintext state
 * With those 5 fields generated, encoder slices them to separate the bits from each field:
 *   xx ssshh ssshC ssshC ssshC sshhC sshhC
 */
 
const char fmn_password_alphabet[32]="ABCDEFGHIJKLMNOPQRSTUVWXYZ134679";

static const uint8_t fmn_password_c_pos[5]={20,15,10,5,0};
static const uint8_t fmn_password_h_pos[9]={1,6,11,16,21,25,2,7,26};
static const uint8_t fmn_password_s_pos[16]={29,24,19,14,9,4,28,23,18,13,8,3,27,22,17,12};
 
static uint32_t fmn_password_hash(uint16_t s) {

  // Calculate 'h' from state (9 bits output).
  uint16_t h=~(((s&0xff00)>>7)^(s&0xff));
  h&=0x1ff;
  
  // Count the ones in 'h' and 's', derive 'c' from that.
  uint16_t q,onec=0;
  for (q=h;q;q>>=1) if (q&1) onec++;
  for (q=s;q;q>>=1) if (q&1) onec++;
  uint16_t c=(32-onec)&0x1f;
  
  // Weave 's', 'h', and 'c' together bit by bit.
  uint32_t bin=0;
  uint8_t i;
  for (i=0;i<sizeof(fmn_password_c_pos);i++) if (c&(1<<i)) bin|=1<<fmn_password_c_pos[i];
  for (i=0;i<sizeof(fmn_password_h_pos);i++) if (h&(1<<i)) bin|=1<<fmn_password_h_pos[i];
  for (i=0;i<sizeof(fmn_password_s_pos);i++) if (s&(1<<i)) bin|=1<<fmn_password_s_pos[i];
  
  return bin;
}

static int8_t fmn_password_unhash(uint16_t *state,uint32_t bin) {

  // It's not possible to arrive here with 'x' nonzero, but in case I'm wrong about that, call it a checksum failure.
  if (bin&0xc0000000) return FMN_PASSWORD_CHECKSUM;

  // Extract the three fields 'c', 'h', 's'.
  uint16_t c=0,h=0,s=0;
  uint8_t i;
  for (i=0;i<sizeof(fmn_password_c_pos);i++) if (bin&(1<<fmn_password_c_pos[i])) c|=1<<i;
  for (i=0;i<sizeof(fmn_password_h_pos);i++) if (bin&(1<<fmn_password_h_pos[i])) h|=1<<i;
  for (i=0;i<sizeof(fmn_password_s_pos);i++) if (bin&(1<<fmn_password_s_pos[i])) s|=1<<i;
  
  // Count the ones in 'h' and 's'.
  uint16_t q,onec=0;
  for (q=h;q;q>>=1) if (q&1) onec++;
  for (q=s;q;q>>=1) if (q&1) onec++;
  
  // Validate checksum.
  c+=onec;
  if (c&0x1f) return FMN_PASSWORD_CHECKSUM;
  
  // Validate hash.
  uint16_t expecth=~(((s&0xff00)>>7)^(s&0xff));
  expecth&=0x1ff;
  if (h!=expecth) return FMN_PASSWORD_CHECKSUM;
  
  // OK valid.
  *state=s;
  return FMN_PASSWORD_OK;
}

static int8_t fmn_password_check_business_rules(uint16_t state) {
  /* XXX 2022-08-07 Disabling business rules as they are making testing very painful...
  int8_t err;
  if (err=fmn_map_validate_region((state&FMN_STATE_LOCATION_MASK)>>FMN_STATE_LOCATION_SHIFT)) return err;
  if (state&FMN_STATE_RESERVED) return FMN_PASSWORD_RESERVED;
  
  // SEQUENCE rules are the diciest.
  // Like, nothing technically prevents the user from having a broom without a feather.
  // But we know from having designed the game that it is not possible in practice.
  // And as a general policy, we want to validate as aggressively as possible.
  
  // The four treasures can only be acquired in a specific order: feather, wand, broom, umbrella
  if ((state&FMN_STATE_WAND)&&!(state&FMN_STATE_FEATHER)) return FMN_PASSWORD_SEQUENCE;
  if ((state&FMN_STATE_BROOM)&&!(state&FMN_STATE_WAND)) return FMN_PASSWORD_SEQUENCE;
  if ((state&FMN_STATE_UMBRELLA)&&!(state&FMN_STATE_BROOM)) return FMN_PASSWORD_SEQUENCE;
  
  // Similarly, some narrative flags are not possible without some precondition.
  if ((state&FMN_STATE_CASTLE_OPEN)&&!(state&FMN_STATE_WAND)) return FMN_PASSWORD_SEQUENCE;
  if ((state&FMN_STATE_WOLF_DEAD)&&!(state&FMN_STATE_CASTLE_OPEN)) return FMN_PASSWORD_SEQUENCE;
  /**/
  
  return FMN_PASSWORD_OK;
}
 
int8_t fmn_password_eval(uint16_t *state,const char *pw/*6*/) {
  if (!state||!pw) return FMN_PASSWORD_ARGUMENT;
  uint32_t bin=0;
  uint8_t shift=0;
  uint8_t i=0;
  for (;i<FMN_PASSWORD_LENGTH;i++,shift+=5) {
    char ch=pw[i];
    uint8_t v;
    
    // A few basic normalization rules.
    if ((ch>='a')&&(ch<='z')) ch-=0x20;
    if (ch=='0') ch='O';
    if (ch=='2') ch='Z';
    if (ch=='5') ch='S';
    if (ch=='8') ch='B';
    
    // A..Z are the first 26 of our alphabet, don't bother searching for them.
    if ((ch>='A')&&(ch<='Z')) v=ch-'A';
    else {
      v=0xff;
      uint8_t p=26; for (;p<32;p++) {
        if (ch==fmn_password_alphabet[p]) {
          v=p;
          break;
        }
      }
      if (v==0xff) return FMN_PASSWORD_ILLEGAL_CHAR;
    }
    
    bin|=v<<shift;
  }
  return fmn_password_unhash(state,bin);
}

int8_t fmn_password_repr(char *pw/*6*/,uint16_t state) {
  if (!pw) return FMN_PASSWORD_ARGUMENT;
  int8_t err=fmn_password_check_business_rules(state);
  if (err<0) return err; // business rules shouldn't fail <0 but allow it
  uint32_t bin=fmn_password_hash(state);
  uint8_t i=0; for (;i<FMN_PASSWORD_LENGTH;i++,bin>>=5) {
    pw[i]=fmn_password_alphabet[bin&0x1f];
  }
  return err;
}

/* Begin.
 */
 
void fmn_password_begin() {
  fbdirty=1;
  memset(entry,'A',sizeof(entry));
  cursorp=0;
}

/* End.
 */
 
void fmn_password_end() {
}

/* Submit password.
 */
 
static void fmn_password_submit() {
  uint16_t state=0;
  int8_t err=fmn_password_eval(&state,entry);
  if (err==FMN_PASSWORD_OK) {
    fmn_game_reset_with_state(state);
    fmn_set_uimode(FMN_UIMODE_PLAY);
  } else {
    fprintf(stderr,"%s: '%.6s' err=%d\n",__func__,entry,err);
  }
}

/* Adjust one character.
 */
 
static void modify_char(int8_t d) {
  uint8_t p=0;
  uint8_t i=0; for (;i<32;i++) if (entry[cursorp]==fmn_password_alphabet[i]) { p=i; break; }
  p+=d;
  p&=0x1f;
  entry[cursorp]=fmn_password_alphabet[p];
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

/* Text input.
 */
 
void fmn_password_text_input(uint32_t codepoint) {
  if ((codepoint>='a')&&(codepoint<='z')) {
    entry[cursorp]=codepoint-0x20;
    if (++cursorp>=FMN_PASSWORD_LENGTH) cursorp=0;
  } else if ((codepoint>='A')&&(codepoint<='Z')) {
    entry[cursorp]=codepoint;
    if (++cursorp>=FMN_PASSWORD_LENGTH) cursorp=0;
  } else if ((codepoint>='0')&&(codepoint<='9')) {
    entry[cursorp]=codepoint;
    if (++cursorp>=FMN_PASSWORD_LENGTH) cursorp=0;
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
  fmn_blit(fb,0,0,&uibits,FMN_NSCOORD(0,24),FMN_NSCOORD(72,40),0);
  int16_t dstx=4,i;
  for (i=0;i<FMN_PASSWORD_LENGTH;i++,dstx+=11) {
    int16_t srcx=0,srcy=64*FMN_GFXSCALE;
    if ((entry[i]>='A')&&(entry[i]<='P')) { // top row
      srcx=(entry[i]-'A')*8*FMN_GFXSCALE;
    } else if ((entry[i]>='Q')&&(entry[i]<='Z')) { // bottom row, left
      srcy+=10*FMN_GFXSCALE;
      srcx=(entry[i]-'Q')*8*FMN_GFXSCALE;
    } else switch (entry[i]) { // six more glyphs on the bottom row (digits)
      case '1': srcy+=10*FMN_GFXSCALE; srcx=80*FMN_GFXSCALE; break;
      case '3': srcy+=10*FMN_GFXSCALE; srcx=88*FMN_GFXSCALE; break;
      case '4': srcy+=10*FMN_GFXSCALE; srcx=96*FMN_GFXSCALE; break;
      case '6': srcy+=10*FMN_GFXSCALE; srcx=104*FMN_GFXSCALE; break;
      case '7': srcy+=10*FMN_GFXSCALE; srcx=112*FMN_GFXSCALE; break;
      case '9': srcy+=10*FMN_GFXSCALE; srcx=120*FMN_GFXSCALE; break;
    }
    fmn_blit(fb,FMN_NSCOORD(dstx,14),&uibits,srcx,srcy,FMN_NSCOORD(8,10),0);
  }
  fmn_blit(fb,FMN_NSCOORD(5+cursorp*11,9),&uibits,FMN_NSCOORD(48+cursorframe*7,0),FMN_NSCOORD(7,5),0);
  fmn_blit(fb,FMN_NSCOORD(5+cursorp*11,24),&uibits,FMN_NSCOORD(48+cursorframe*7,5),FMN_NSCOORD(7,5),0);
}
