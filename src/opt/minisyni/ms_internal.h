#ifndef MS_INTERNAL_H
#define MS_INTERNAL_H

#include "minisyni.h"
#include <string.h>
#include <stdio.h>

#define MS_VOICE_LIMIT 8

extern struct minisyni {
  uint16_t rate;
  uint8_t chanc;
  uint16_t default_ttl;
  
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
