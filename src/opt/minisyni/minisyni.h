/* minisyni.h
 * Synthesizer optimized for low memory and high performance, should be suitable for Arduino.
 */
 
#ifndef MINISYNI_H
#define MINISYNI_H

#include <stdint.h>

/* We don't allocate memory, so no cleanup is necessary, and
 * failure is only possible for bad params.
 * Mono only, but we'll repeat the signal on multiple channels eg if you can only open a stereo output.
 * Requires (rate>0) and (chanc>0).
 */
void minisyni_init(uint16_t rate,uint8_t chanc);

/* Generate (c) samples beginning at (v).
 * Always *samples* regardless of channel count. Must be a multiple of (chanc).
 */
int minisyni_update(int16_t *v,int32_t c);

//TODO actually do stuff

#endif
