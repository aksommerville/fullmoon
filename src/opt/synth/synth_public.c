#include "synth_internal.h"
#include <math.h>

struct synth synth={0};

/* Global rates table.
 * Leave the reference rate at 22050; that's Tiny's rate and we don't want lil Tiny recalculating.
 */

static uint32_t synth_rate_reference=22050;
static uint32_t synth_ratev[128]={
  2125742,2252146,2386065,2527948,2678268,2837526,3006254,3185015,3374406,3575058,3787642,4012867,4251485,
  4504291,4772130,5055896,5356535,5675051,6012507,6370030,6748811,7150117,7575285,8025735,8502970,9008582,
  9544261,10111792,10713070,11350103,12025015,12740059,13497623,14300233,15150569,16051469,17005939,18017165,
  19088521,20223584,21426141,22700205,24050030,25480119,26995246,28600467,30301139,32102938,34011878,36034330,
  38177043,40447168,42852281,45400411,48100060,50960238,53990491,57200933,60602276,64205876,68023757,72068660,
  76354085,80894335,85704563,90800821,96200119,101920476,107980983,114401866,121204555,128411753,136047513,
  144137319,152708170,161788671,171409126,181601643,192400238,203840952,215961966,228803732,242409110,256823506,
  272095026,288274639,305416341,323577341,342818251,363203285,384800477,407681904,431923931,457607465,484818220,
  513647012,544190053,576549277,610832681,647154683,685636503,726406571,769600953,815363807,863847862,915214929,
  969636441,1027294024,1088380105,1153098554,1221665363,1294309365,1371273005,1452813141,1539201906,1630727614,
  1727695724,1830429858,1939272882,2054588048,2176760211,2306197109,2443330725,2588618730,2742546010,2905626283,
  3078403812,3261455229,
};

/* Init.
 */
 
void synth_init(uint32_t rate) {
  if (rate<SYNTH_RATE_MIN) rate=SYNTH_RATE_MIN;
  else if (rate>SYNTH_RATE_MAX) rate=SYNTH_RATE_MAX;
  
  if (rate!=synth_rate_reference) {
    float scale=(float)rate/(float)synth_rate_reference;
    uint32_t *v=synth_ratev;
    uint8_t i=128;
    for (;i-->0;v++) {
      *v=((*v)*scale);
    }
    synth_rate_reference=rate;
  }
  
  synth.rate=rate;
  
  synth_reset();
}

/* Update voices.
 */
 
static void synth_update_voices(int16_t *v,int c) {
  struct synth_voice *voice=synth.voicev;
  uint8_t i=synth.voicec;
  for (;i-->0;voice++) {
    if (!voice->update) continue;
    if (voice->ttl) {
      if (voice->ttl<=c) {
        if (voice->release) voice->release(voice);
        else voice->update=0;
        if (!voice->update) continue;
      } else {
        voice->ttl-=c;
      }
    }
    voice->update(v,c,voice);
  }
}

/* Update song.
 * Call only when (songdelay==0).
 * Processes the next event and advances (songp), possibly looping.
 * Sets (songdelay) as needed. Caller should repeat until it becomes nonzero or (song) goes null.
 * If we loop, we always delay one extra frame at the moment of the loop, to guarantee that we can't get stuck forever.
 */
 
static void synth_update_song() {
  
  // End of song?
  if (synth.songp>=synth.songc) {
   _eof_:;
    synth_release_all();
    if (synth.songrepeat) {
      synth.songp=(synth.song[4]<<8)|synth.song[5];
      synth.songdelay=1;
    } else {
      synth.song=0;
      synth.songdelay=0;
    }
    return;
  }
  
  // First byte tells us the opcode and implicitly the payload length.
  uint8_t lead=synth.song[synth.songp++];
  
  // Full zero is EOF.
  if (!lead) goto _eof_;
  
  // High bit zero, the rest is a delay in ticks.
  if (!(lead&0x80)) {
    synth.songdelay=synth.songtempo*lead;
    return;
  }
  
  // The rest are distinguished by their top 4 bits.
  uint8_t a,b;
  #define PAYLEN(c) { \
    if (synth.songp>synth.songc-c) { \
      synth.song=0; \
      return; \
    } \
    if (c>=1) { \
      a=synth.song[synth.songp++]; \
      if (c>=2) { \
        b=synth.song[synth.songp++]; \
      } \
    } \
  }
  switch (lead&0xf0) {
    case 0x80: PAYLEN(2) synth_event_note_off(lead&0x0f,a,b); break;
    case 0x90: PAYLEN(2) synth_event_note_on(lead&0x0f,a,b); break;
    case 0xa0: PAYLEN(2) synth_event_note_adjust(lead&0x0f,a,b); break;
    case 0xb0: PAYLEN(2) synth_event_control(lead&0x0f,a,b); break;
    case 0xc0: PAYLEN(1) synth_event_program(lead&0x0f,a); break;
    case 0xd0: PAYLEN(1) synth_event_pressure(lead&0x0f,a); break;
    case 0xe0: PAYLEN(2) synth_event_wheel(lead&0x0f,(a|(b<<7))-8192); break;
    case 0xf0: PAYLEN(2) {
        uint8_t waveid=(lead>>1)&7;
        uint8_t noteid=b&0x7f;
        uint32_t ttl=(b>>7)|(a<<1)|((lead&1)<<9);
        ttl*=synth.songtempo;
        if (ttl>0xffff) ttl=0xffff;
        synth_event_fireforget(waveid,noteid,ttl);
      } break;
  }
  #undef PAYLEN
}

/* Update with a running song.
 */
 
static void synth_update_with_song(int16_t *v,int32_t c) {
  while (c>0) {
  
    // Process song events until it acquires a delay.
    while (synth.song&&!synth.songdelay) {
      synth_update_song();
    }
    
    // Song vanished. No worries, just run to completion.
    if (synth.songdelay<1) {
      synth_update_voices(v,c);
      return;
    }
    
    // Advance time by the smaller of (c,songdelay).
    int32_t updc=synth.songdelay;
    if (updc>c) updc=c;
    synth_update_voices(v,updc);
    v+=updc;
    c-=updc;
    synth.songdelay-=updc;
  }
}

/* Update.
 */

void synth_update(int16_t *v,int c) {
  if (c<1) return;
  memset(v,0,c<<1);
  if (synth.song) synth_update_with_song(v,c);
  else synth_update_voices(v,c);
  while (synth.voicec&&!synth.voicev[synth.voicec-1].update) synth.voicec--;
}

/* Begin song.
 */
 
int8_t synth_play_song(const void *v,uint16_t c,uint8_t force,uint8_t repeat) {
  
  // Request for the current song without force is special.
  if ((v==synth.song)&&(c==synth.songc)&&!force) {
    synth.songrepeat=repeat;
    return 0;
  }
  
  // Requesting an empty song, ie none, is always legal.
  if (!c) {
    if (!synth.song) return 0;
    synth_release_all();
    synth.song=0;
    synth.songp=0;
    synth.songc=0;
    synth.songdelay=0;
    return 1;
  }
  
  // Validate.
  if (!v||(c<6)) return -1;
  const uint8_t *SRC=v;
  uint16_t tempo=(SRC[0]<<8)|SRC[1];
  uint16_t startp=(SRC[2]<<8)|SRC[3];
  uint16_t loopp=(SRC[4]<<8)|SRC[5];
  if (startp<6) return -1;
  if (loopp<startp) return -1;
  if (loopp>=c) return -1;
  
  // Drop existing voices.
  synth_release_all();
  
  // Start the new song, the easy part.
  synth.song=v;
  synth.songc=c;
  synth.songp=startp;
  synth.songdelay=0;
  synth.songrepeat=repeat;
  
  // Calculate the new tempo, minimum 1 frame/tick.
  // There is a real danger of overflow here, so cast to float first.
  synth.songtempo=((float)tempo*(float)synth.rate)/1000000.0f;
  if (synth.songtempo<1) synth.songtempo=1;
  
  return 1;
}

/* Three flavors of reset.
 */
 
void synth_reset() {
  synth.voicec=0;
  struct synth_channel *channel=synth.channelv;
  uint8_t i=0;
  for (;i<SYNTH_CHANNEL_COUNT;i++,channel++) synth_channel_default(channel,i);
}

void synth_silence() {
  synth.voicec=0;
}

void synth_release_all() {
  struct synth_voice *voice=synth.voicev;
  uint8_t i=synth.voicec;
  for (;i-->0;voice++) {
    if (voice->release) {
      // Most voice types should set a release hook and take care of it.
      voice->release(voice);
    } else if ((voice->chid!=0xff)||(voice->noteid!=0xff)) {
      // No release hook but it also claims to be addressable? Kill it now.
      voice->update=0;
    }
    voice->chid=0xff;
    voice->noteid=0xff;
  }
}

/* Realtime event.
 */

void synth_event_realtime(uint8_t e) {
  switch (e) {
    case 0xf8: break; // Timing Clock.
    case 0xfa: break; // Timing Start.
    case 0xfb: break; // Timing Continue.
    case 0xfc: break; // Timing Stop.
    case 0xfe: break; // Active Sensing.
    case 0xff: synth_reset(); break; // Reset.
  }
}

/* Note Off.
 */
 
void synth_event_note_off(uint8_t chid,uint8_t noteid,uint8_t velocity) {
  struct synth_voice *voice=synth.voicev;
  uint8_t i=synth.voicec;
  for (;i-->0;voice++) {
    if (voice->chid!=chid) continue;
    if (voice->noteid!=noteid) continue;
    if (voice->release) voice->release(voice);
    else voice->update=0;
    voice->chid=0xff;
    voice->noteid=0xff;
    return;
  }
}

/* Note On.
 */
 
static void synth_update_dummy(int16_t *v,int c,struct synth_voice *voice) {}
 
void *synth_event_note_on(uint8_t chid,uint8_t noteid,uint8_t velocity) {

  // Require a valid chid and noteid. velocity not so much.
  if (chid>=SYNTH_CHANNEL_COUNT) return 0;
  if (noteid>=0x80) return 0;
  struct synth_channel *channel=synth.channelv+chid;
  
  //TODO Monophonic channels, do we want them?

  // Reject if already playing. Record slots we might take over.
  struct synth_voice *available=0; // Not currently playing.
  struct synth_voice *unaddressable=0; // Unaddressable, presumably winding down.
  struct synth_voice *voice=synth.voicev;
  uint8_t i=synth.voicec;
  for (;i-->0;voice++) {
    if (!voice->update) {
      available=voice;
      continue;
    }
    if ((voice->chid==chid)&&(voice->noteid==noteid)) {
      return 0;
    }
    if ((voice->chid==0xff)&&(voice->noteid==0xff)) {
      unaddressable=voice;
    }
  }
  
  // Select a slot for the new voice.
  if (available) voice=available;
  else if (synth.voicec<SYNTH_VOICE_LIMIT) {
    synth.voicev[synth.voicec].update=synth_update_dummy; // in case the callback is running (TODO we need a safer plan)
    voice=synth.voicev+synth.voicec++;
  } else if (unaddressable) voice=unaddressable;
  else return 0;
  
  // Set it up.
  voice->chid=chid;
  voice->noteid=noteid;
  voice->noterate=synth_ratev[noteid];
  voice->rate=synth_bend_rate(voice->noterate,channel->wheel,channel->wheelrange);
  voice->p=0;
  voice->ttl=0;
  synth_voice_setup(voice,noteid,velocity,channel);
  return voice;
}

/* Fire-and-forget note.
 */
 
void synth_event_fireforget(uint8_t chid,uint8_t noteid,uint32_t ttl) {
  struct synth_voice *voice=synth_event_note_on(chid,noteid,0x40);
  if (!voice) return;
  voice->ttl=ttl;
  voice->chid=0xff;
  voice->noteid=0xff;
}

/* Note Adjust.
 */
 
void synth_event_note_adjust(uint8_t chid,uint8_t noteid,uint8_t velocity) {
  struct synth_voice *voice=synth.voicev;
  uint8_t i=synth.voicec;
  for (;i-->0;voice++) {
    if (voice->chid!=chid) continue;
    if (voice->noteid!=noteid) continue;
    if (voice->adjust) voice->adjust(voice,velocity);
    return;
  }
}

/* Control Change.
 */
 
void synth_event_control(uint8_t chid,uint8_t k,uint8_t v) {
  if (chid>=SYNTH_CHANNEL_COUNT) return;
  struct synth_channel *channel=synth.channelv+chid;
  switch (k) {
    case 0x01: channel->mod=v; break;
    case 0x07: channel->volume=v; break;
  }
}

/* Program Change.
 */
 
void synth_event_program(uint8_t chid,uint8_t pid) {
  if (chid>=SYNTH_CHANNEL_COUNT) return;
  struct synth_channel *channel=synth.channelv+chid;
  channel->pid=pid;
}

/* Channel Pressure.
 */
 
void synth_event_pressure(uint8_t chid,uint8_t v) {
  struct synth_voice *voice=synth.voicev;
  uint8_t i=synth.voicec;
  for (;i-->0;voice++) {
    if (voice->chid!=chid) continue;
    if (voice->adjust) voice->adjust(voice,v);
  }
}

/* Pitch Wheel.
 */
 
void synth_event_wheel(uint8_t chid,uint16_t v) {
  if (chid>=SYNTH_CHANNEL_COUNT) return;
  struct synth_channel *channel=synth.channelv+chid;
  if (v==channel->wheel) return;
  channel->wheel=v;
  if (!channel->wheelrange) return;
  struct synth_voice *voice=synth.voicev;
  uint8_t i=synth.voicec;
  for (;i-->0;voice++) {
    if (voice->chid!=chid) continue;
    voice->rate=synth_bend_rate(voice->noterate,v,channel->wheelrange);
  }
}

/* Bend rate.
 */
 
uint32_t synth_bend_rate(uint32_t base,uint16_t bend,uint16_t range) {
  if (bend==0x2000) return base;
  if (!range) return base;
  float fbend=((float)(bend-0x2000)*(float)range)/(8192.0f*1200.0f);
  return (uint32_t)((float)base*powf(2.0f,fbend));
}
