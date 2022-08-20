#include "synth_internal.h"

#define envstage voice->bv[0]
#define releasing voice->bv[1]
#define modrate voice->bv[2] /* 0x10+log2(relative modulator rate) */
#define envc voice->lv[0]
#define envd voice->lv[1]
#define level voice->lv[2]
#define decc voice->lv[3]
#define susl voice->lv[4]
#define rlsc voice->lv[5]
#define p0 voice->lv[7]
#define wave0lv voice->pv[0]
#define wave0 ((int16_t*)voice->pv[0])
#define wave1lv voice->pv[1]
#define wave1 ((int16_t*)voice->pv[1])
#define chanlv voice->pv[2]
#define chan ((struct synth_channel*)voice->pv[2])

/* Update.
 */
 
void synth_fm_update(int16_t *v,int c,struct synth_voice *voice) {
  uint32_t range=chan->mod<<10;
  uint32_t moddp=voice->rate;
  if (modrate<0x10) moddp>>=(0x10-modrate);
  else moddp<<=(modrate-0x10);
  for (;c-->0;v++) {
  
    voice->p+=voice->rate;
    
    uint32_t p=voice->p>>SYNTH_WAVE_P_SHIFT;
    int32_t sample;
    
    p0+=moddp;
    int16_t mod=wave0[((uint32_t)p0)>>SYNTH_WAVE_P_SHIFT];
    uint32_t p1=voice->p+voice->rate+mod*range;
    sample=wave1[p1>>SYNTH_WAVE_P_SHIFT];
    
    if (envc>0) {
      envc--;
      level+=envd;
    } else switch (envstage) {
      case 0: {
          envstage=1;
          envc=decc;
          envd=(susl-level)/envc;
        } break;
      case 1: {
          envstage=2;
          envc=100;
          envd=0;
        } break;
      case 2: {
          if (releasing) {
            envstage=3;
            envc=rlsc;
            envd=-level/envc;
          } else {
            envc=100;
          }
        } break;
      case 3: {
          voice->update=0;
          return;
        } break;
    }
    
    sample=(sample*(level>>16))>>15;
    
    (*v)+=sample;
  }
}

/* Release.
 */
 
void synth_fm_release(struct synth_voice *voice) {
  releasing=1;
}

/* Setup.
 */
 
void synth_fm_setup(struct synth_voice *voice,uint8_t noteid,uint8_t velocity,struct synth_channel *channel) {
  voice->update=synth_fm_update;
  voice->release=synth_fm_release;
  voice->adjust=0;
  wave0lv=(void*)synth_wave_4;
  wave1lv=(void*)synth_wave_4;
  envstage=0;
  level=0;
  releasing=0;
  chanlv=channel;
  p0=0;
  modrate=0x10;
  
  int32_t attack_time=(channel->atktlo*(0x7f-velocity)+channel->atkthi*velocity)>>7;
  int32_t release_time=(channel->rlstlo*(0x7f-velocity)+channel->rlsthi*velocity)>>7;
  int32_t attack_level=channel->volume<<23;
  int32_t decay_time=attack_time*2;
  int32_t sustain_level=(attack_level>>7)*channel->sustainlevel;
  
  envc=attack_time;
  envd=attack_level/envc;
  decc=decay_time;
  susl=sustain_level;
  rlsc=release_time;
}
