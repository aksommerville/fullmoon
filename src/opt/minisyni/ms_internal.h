#ifndef MS_INTERNAL_H
#define MS_INTERNAL_H

#include "minisyni.h"
#include <string.h>

extern struct minisyni {
  uint16_t rate;
  uint8_t chanc;
  
  //XXX TEMP generate a wave just for a lights-on test.
  uint32_t tmpp;
  uint32_t tmppd;
  int16_t tmplevel;
} minisyni;

#endif
