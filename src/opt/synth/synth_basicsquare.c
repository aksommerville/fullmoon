#include "synth_internal.h"

#define level voice->sv[0]
#define vello voice->sv[1]
#define velhi voice->sv[2]

/* Update.
 */
 
void synth_basicsquare_update(int16_t *v,int c,struct synth_voice *voice) {
  for (;c-->0;v++) {
    voice->p+=voice->rate;
    if (voice->p&0x80000000) (*v)-=level;
    else (*v)+=level;
  }
}

/* Release.
 */
 
void synth_basicsquare_release(struct synth_voice *voice) {
  voice->update=0;
}

/* Adjust.
 */
 
void synth_basicsquare_adjust(struct synth_voice *voice,uint8_t velocity) {
  if (velocity<=1) {
    level=vello;
  } else if (velocity>=0x7f) {
    level=velhi;
  } else {
    level=(vello*(0x7f-velocity)+velhi*velocity)>>7;
  }
}

/* Setup.
 */
 
void synth_basicsquare_setup(struct synth_voice *voice,uint8_t noteid,uint8_t velocity,struct synth_channel *channel) {
  voice->update=synth_basicsquare_update;
  voice->release=synth_basicsquare_release;
  voice->adjust=synth_basicsquare_adjust;
  vello=(channel->vlo*channel->volume)>>7;
  velhi=(channel->vhi*channel->volume)>>7;
  synth_basicsquare_adjust(voice,velocity);
}
