#include "synth_internal.h"

/* Set channel to initial defaults.
 */
 
void synth_channel_default(struct synth_channel *channel,uint8_t chid) {
  channel->pid=chid;
  channel->volume=0x40;
  channel->wheel=0x2000;
  channel->wheelrange=200;
  channel->vlo=0x1000;
  channel->vhi=0x2000;
  channel->atktlo=( 50*synth.rate)/1000;
  channel->atkthi=(  5*synth.rate)/1000;
  channel->rlstlo=( 50*synth.rate)/1000;
  channel->rlsthi=(300*synth.rate)/1000;
  channel->sustainlevel=0x40;
  channel->mod=0;
}

/* Setup voice.
 */
 
void synth_voice_setup(struct synth_voice *voice,uint8_t noteid,uint8_t velocity,struct synth_channel *channel) {
  voice->update=0;
  switch (channel->pid&3) {
    case 0: synth_basicsquare_setup(voice,noteid,velocity,channel); return;
    case 1: synth_wave_setup(voice,noteid,velocity,channel); return;
    case 2: synth_mix_setup(voice,noteid,velocity,channel); return;
    case 3: synth_fm_setup(voice,noteid,velocity,channel); return;
  }
}
