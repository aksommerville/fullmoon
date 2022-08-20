#ifndef SYNTH_INTERNAL_H
#define SYNTH_INTERNAL_H

#include "synth.h"
#include <string.h>
#include <stdio.h>

#define SYNTH_RATE_MIN 200
#define SYNTH_RATE_MAX 200000
#define SYNTH_CHANNEL_COUNT 16 /* Should be 16 to agree with MIDI. */
#define SYNTH_VOICE_LIMIT 16 /* Keep under 256, otherwise highly tweakable. */
#define SYNTH_BV_SIZE 4
#define SYNTH_SV_SIZE 4
#define SYNTH_LV_SIZE 8
#define SYNTH_PV_SIZE 4
#define SYNTH_WAVE_SIZE_BITS 9
#define SYNTH_WAVE_SIZE_SAMPLES (1<<SYNTH_WAVE_SIZE_BITS)
#define SYNTH_WAVE_P_SHIFT (32-SYNTH_WAVE_SIZE_BITS)

//TODO I don't want the wave data living in synth, it's an app concern.
// Also, this data is highly temporary, just copied out of Pocket Orchestra.
extern const int16_t synth_wave_0[];
extern const int16_t synth_wave_1[];
extern const int16_t synth_wave_2[];
extern const int16_t synth_wave_3[];
extern const int16_t synth_wave_4[];

extern struct synth {
  uint32_t rate;
  
  // We hold on to the raw serial song, headers and all.
  const uint8_t *song;
  uint16_t songc;
  uint16_t songp;
  int32_t songdelay; // frames until the next song command
  uint8_t songrepeat; // 0,1
  uint16_t songtempo; // frames/tick, minimum 1 if a song is present
  
  struct synth_channel {
    uint8_t pid;
    uint8_t volume; // default 0x40, MIDI Control 0x07
    uint16_t wheel; // default 0x2000
    uint16_t wheelrange; // cents, default 200
    int16_t vlo,vhi; // velocity range, precise meaning is up to the implementation. Default 0x1000..0x2000
    int32_t atktlo,atkthi; // attack time, range per velocity
    int32_t rlstlo,rlsthi; // '' release
    uint8_t sustainlevel; // 0..0x7f; sustain level relative to attack level
    uint8_t mod; // 0..0x7f, mod wheel straight off the bus
  } channelv[SYNTH_CHANNEL_COUNT];
  
  struct synth_voice {
  
    // (update) is required. If unset, the voice is unused.
    // If (release) unset, the voice must self-terminate in a reasonable time.
    void (*update)(int16_t *v,int c,struct synth_voice *voice);
    void (*release)(struct synth_voice *voice);
    void (*adjust)(struct synth_voice *voice,uint8_t velocity); // Note Adjust or Channel Pressure
    
    // For identification only. 0xff if not addressable.
    uint8_t chid,noteid;
    
    uint32_t noterate; // Before bend.
    uint32_t rate; // Actual playback rate.
    uint32_t ttl;
    
    // Everything else, usage is entirely up to the implementation.
    uint32_t p;
    uint8_t bv[SYNTH_BV_SIZE];
    int16_t sv[SYNTH_SV_SIZE];
    int32_t lv[SYNTH_LV_SIZE];
    void *pv[SYNTH_PV_SIZE];
    
  } voicev[SYNTH_VOICE_LIMIT];
  uint8_t voicec;
  
} synth;

/* (base) and output in dp units.
 * (bend) in 0..0x3fff, eg MIDI Pitch Wheel payload.
 * (range) in cents.
 */
uint32_t synth_bend_rate(uint32_t base,uint16_t bend,uint16_t range);

void synth_channel_default(struct synth_channel *channel,uint8_t chid);

/* Each voice implementation consists of 4 hooks:
 *  - setup(voice,noteid,velocity)
 *      Voice is already prepared (eg rate,noterate).
 *      Initialize your extension fields.
 *      This is the only place velocity can be applied, we don't touch it generically.
 *  - update(v,c,voice)
 *      REQUIRED. Add to (v). Unset (voice->update) if you expire.
 *  - release(voice)
 *      OPTIONAL. Hard stops only if not implemented.
 *  - adjust(voice,velocity)
 *      OPTIONAL. Both Note Adjust and Channel Pressure.
 */
 
void synth_voice_setup(struct synth_voice *voice,uint8_t noteid,uint8_t velocity,struct synth_channel *channel);

void synth_basicsquare_setup(struct synth_voice *voice,uint8_t noteid,uint8_t velocity,struct synth_channel *channel);
void synth_wave_setup(struct synth_voice *voice,uint8_t noteid,uint8_t velocity,struct synth_channel *channel);
void synth_mix_setup(struct synth_voice *voice,uint8_t noteid,uint8_t velocity,struct synth_channel *channel);
void synth_fm_setup(struct synth_voice *voice,uint8_t noteid,uint8_t velocity,struct synth_channel *channel);

//TODO square with envelope
//TODO two waves with mix LFO
//TODO two waves with mix envelope

#endif
