#include "opt/minisyni/ms_internal.h"

#define MS_P_SHIFT (32-9)

struct minisyni minisyni={0};

#if FMN_USE_web
  // Hard to pass pointers across the Wasm boundary.
  // Much simpler if we can just allocate the buffer here.
  int16_t storage_for_wasi[512];
#endif

static uint32_t minisyni_ratev[128]={
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
static uint16_t minisyni_ratev_base=22050;

static int16_t TMP_wave[512];
#include <math.h>

/* Init.
 */
 
void minisyni_init(uint16_t rate) {

  minisyni.rate=rate;
  minisyni.default_ttl=rate/10;
  
  if (rate!=minisyni_ratev_base) {
    uint32_t *v=minisyni_ratev;
    uint8_t i=0x80;
    for (;i-->0;v++) *v=((*v)*rate)/minisyni_ratev_base;
    minisyni_ratev_base=rate;
  }
  
  {
    int16_t *v=TMP_wave;
    int i=512;
    //for (;i-->0;v++) *v=(int16_t)(sinf((i*M_PI*2.0f)/512.0f)*5000.0f);
    for (;i-->0;v++) *v=(i*10000)/512-5000;
  }
}

/* Update song.
 * Call only when (songdelay==0).
 * Processes the next event and advances (songp), possibly looping.
 * Sets (songdelay) as needed. Caller should repeat until it becomes nonzero or (song) goes null.
 * If we loop, we always delay one extra frame at the moment of the loop, to guarantee that we can't get stuck forever.
 */
 
static void ms_update_song() {
  
  // End of song?
  if (minisyni.songp>=minisyni.songc) {
   _eof_:;
    minisyni_release_all();
    if (minisyni.songrepeat) {
      minisyni.songp=(minisyni.song[4]<<8)|minisyni.song[5];
      minisyni.songdelay=1;
    } else {
      minisyni.song=0;
      minisyni.songdelay=0;
    }
    return;
  }
  
  // First byte tells us the opcode and implicitly the payload length.
  uint8_t lead=minisyni.song[minisyni.songp++];
  
  // Full zero is EOF.
  if (!lead) goto _eof_;
  
  // High bit zero, the rest is a delay in ticks.
  if (!(lead&0x80)) {
    minisyni.songdelay=minisyni.songtempo*lead;
    return;
  }
  
  // The rest are distinguished by their top 4 bits.
  uint8_t a,b;
  #define PAYLEN(c) { \
    if (minisyni.songp>minisyni.songc-c) { \
      minisyni.song=0; \
      return; \
    } \
    if (c>=1) { \
      a=minisyni.song[minisyni.songp++]; \
      if (c>=2) { \
        b=minisyni.song[minisyni.songp++]; \
      } \
    } \
  }
  switch (lead&0xf0) {
    case 0x80: PAYLEN(2) minisyni_note_off(lead&0x0f,a,b); break;
    case 0x90: PAYLEN(2) minisyni_note_on(lead&0x0f,a,b); break;
    case 0xa0: PAYLEN(2) minisyni_note_adjust(lead&0x0f,a,b); break;
    case 0xb0: PAYLEN(2) minisyni_control_change(lead&0x0f,a,b); break;
    case 0xc0: PAYLEN(1) minisyni_program_change(lead&0x0f,a); break;
    case 0xd0: PAYLEN(1) minisyni_channel_pressure(lead&0x0f,a); break;
    case 0xe0: PAYLEN(2) minisyni_pitch_wheel(lead&0x0f,(a|(b<<7))-8192); break;
    case 0xf0: PAYLEN(2) {
        uint8_t waveid=(lead>>1)&7;
        uint8_t noteid=b&0x7f;
        uint32_t ttl=(b>>7)|(a<<1)|((lead&1)<<9);
        ttl*=minisyni.songtempo;
        if (ttl>0xffff) ttl=0xffff;
        minisyni_fire_forget(waveid,noteid,ttl);
      } break;
  }
  #undef PAYLEN
}

/* Update one voice. Add to buffer.
 */
 
static void ms_voice_update(int16_t *v,int32_t c,struct ms_voice *voice) {
  for (;c-->0;v++) {
    if (voice->ttl) {
      voice->ttl--;
      if (!voice->ttl) {
        voice->src=0;
        return;
      }
      if (voice->ttl<minisyni.default_ttl) {
        (*v)+=(voice->src[voice->p>>MS_P_SHIFT]*voice->ttl)/minisyni.default_ttl;
      } else {
        (*v)+=voice->src[voice->p>>MS_P_SHIFT];
      }
    } else {
      (*v)+=voice->src[voice->p>>MS_P_SHIFT];
    }
    voice->p+=voice->pd;
  }
}

/* Update all voices.
 * Caller must zero the buffer first, and deal with the song.
 */

static void ms_update_voices(int16_t *v,int32_t c) {
  struct ms_voice *voice=minisyni.voicev;
  uint8_t i=minisyni.voicec;
  for (;i-->0;voice++) {
    if (!voice->src) continue;
    ms_voice_update(v,c,voice);
  }
}

/* Update with a running song.
 */
 
static void ms_update_with_song(int16_t *v,int32_t c) {
  while (c>0) {
  
    // Process song events until it acquires a delay.
    while (minisyni.song&&!minisyni.songdelay) {
      ms_update_song();
    }
    
    // Song vanished. No worries, just run to completion.
    if (minisyni.songdelay<1) {
      ms_update_voices(v,c);
      return;
    }
    
    // Advance time by the smaller of (c,songdelay).
    int32_t updc=minisyni.songdelay;
    if (updc>c) updc=c;
    ms_update_voices(v,updc);
    v+=updc;
    c-=updc;
    minisyni.songdelay-=updc;
  }
}

/* Update.
 */

void minisyni_update(int16_t *v,int32_t c) {
  if (c<1) return;
  memset(v,0,c<<1);
  if (minisyni.song) ms_update_with_song(v,c);
  else ms_update_voices(v,c);
}

/* Begin song.
 */
 
int8_t minisyni_play_song(const uint8_t *v,uint16_t c,uint8_t force,uint8_t repeat) {
  
  // Request for the current song without force is special.
  if ((v==minisyni.song)&&(c==minisyni.songc)&&!force) {
    minisyni.songrepeat=repeat;
    return 0;
  }
  
  // Requesting an empty song, ie none, is always legal.
  if (!c) {
    if (!minisyni.song) return 0;
    minisyni_release_all();
    minisyni.song=0;
    minisyni.songp=0;
    minisyni.songc=0;
    minisyni.songdelay=0;
    return 1;
  }
  
  // Validate.
  if (!v||(c<6)) return -1;
  uint16_t tempo=(v[0]<<8)|v[1];
  uint16_t startp=(v[2]<<8)|v[3];
  uint16_t loopp=(v[4]<<8)|v[5];
  if (startp<6) return -1;
  if (loopp<startp) return -1;
  if (loopp>=c) return -1;
  
  // Drop existing voices.
  minisyni_release_all();
  
  // Start the new song, the easy part.
  minisyni.song=v;
  minisyni.songc=c;
  minisyni.songp=startp;
  minisyni.songdelay=0;
  minisyni.songrepeat=repeat;
  
  // Calculate the new tempo, minimum 1 frame/tick.
  // There is a real danger of overflow here, so cast to float first.
  minisyni.songtempo=((float)tempo*(float)minisyni.rate)/1000000.0f;
  if (minisyni.songtempo<1) minisyni.songtempo=1;
  
  return 1;
}

/* All-off, two variations.
 */
 
void minisyni_release_all() {
  struct ms_voice *voice=minisyni.voicev;
  uint8_t i=minisyni.voicec;
  for (;i-->0;voice++) {
    if (!voice->src) continue;
    voice->chid=0xff;
    voice->noteid=0xff;
    if (!voice->ttl||(voice->ttl>minisyni.default_ttl)) {
      voice->ttl=minisyni.default_ttl;
    }
  }
}

void minisyni_silence_all() {
  minisyni.voicec=0;
}

/* Rate from MIDI note.
 */
 
static uint32_t ms_normrate_from_midi_note(uint8_t noteid) {
  if (noteid>0x7f) noteid=0x7f;
  return minisyni_ratev[noteid];
}

/* Get an unused voice for a new note.
 */
 
static struct ms_voice *ms_unused_voice() {
  struct ms_voice *voice=minisyni.voicev;
  uint8_t i=minisyni.voicec;
  for (;i-->0;voice++) {
    if (voice->src) continue;
    return voice;
  }
  if (minisyni.voicec<MS_VOICE_LIMIT) {
    voice=minisyni.voicev+minisyni.voicec++;
    voice->src=0;
    return voice;
  }
  //TODO Should we kick out an old voice?
  return 0;
}

/* Fire-and-forget note.
 */
 
void minisyni_fire_forget(uint8_t waveid,uint8_t noteid,uint16_t ttl) {
  struct ms_voice *voice=ms_unused_voice();
  if (!voice) return;
  voice->chid=0xff;
  voice->noteid=0xff; // sic, (voice->noteid) is for id only
  voice->src=TMP_wave;//TODO select wave
  voice->p=0;
  voice->pd=ms_normrate_from_midi_note(noteid);
  voice->ttl=ttl;
}

/* Note on.
 */
 
void minisyni_note_on(uint8_t chid,uint8_t noteid,uint8_t velocity) {
  struct ms_voice *voice=ms_unused_voice();
  if (!voice) return;
  voice->chid=chid;
  voice->noteid=noteid;
  voice->src=TMP_wave;//TODO select wave
  voice->p=0;
  voice->pd=ms_normrate_from_midi_note(noteid);
  voice->ttl=0;
}

/* Note off.
 * We don't check for uniqueness at note_on, so there can be multiple addressable voices with the same ID.
 * Maybe the correct thing to do then would be just knock off one of them?
 * Or not let the redundant note begin in the first place?
 * Whatever. We deal with it by releasing all voices that match, at Note Off.
 */
 
void minisyni_note_off(uint8_t chid,uint8_t noteid,uint8_t velocity) {
  struct ms_voice *voice=minisyni.voicev;
  uint8_t i=minisyni.voicec;
  for (;i-->0;voice++) {
    if (voice->chid!=chid) continue;
    if (voice->noteid!=noteid) continue;
    voice->chid=0xff;
    voice->noteid=0xff;
    voice->ttl=minisyni.default_ttl;
  }
}

/* Events we ignore.
 */
 
void minisyni_note_adjust(uint8_t chid,uint8_t noteid,uint8_t velocity) {}
void minisyni_control_change(uint8_t chid,uint8_t key,uint8_t value) {}
void minisyni_program_change(uint8_t chid,uint8_t pid) {}
void minisyni_channel_pressure(uint8_t chid,uint8_t pressure) {}
void minisyni_pitch_wheel(uint8_t chid,int16_t v) {}
