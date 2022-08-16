#ifndef MS_INTERNAL_H
#define MS_INTERNAL_H

#include "minisyni.h"
#include <string.h>
#include <stdio.h>

#define MS_VOICE_LIMIT 8

extern struct minisyni {
  uint16_t rate;
  uint16_t default_ttl;
  
  // We hold on to the raw serial song, headers and all.
  const uint8_t *song;
  uint16_t songc;
  uint16_t songp;
  int32_t songdelay; // frames until the next song command
  uint8_t songrepeat; // 0,1
  uint16_t songtempo; // frames/tick, minimum 1 if a song is present
  
  struct ms_voice {
    const int16_t *src;
    uint32_t p;
    uint32_t pd;
    uint16_t ttl; // maybe temp? in lieu of envelopes
    //TODO level envelope
    //TODO pitch bend? say only linear and constant?
    uint8_t chid,noteid; // For identification. 0xff if unaddressable.
  } voicev[MS_VOICE_LIMIT];
  uint8_t voicec;
  
} minisyni;

#endif
