/* minisyni.h
 * Synthesizer optimized for low memory and high performance, should be suitable for Arduino.
 * XXX Dropping this and replacing with 'synth' -- mostly the same thing.
 */
 
#ifndef MINISYNI_H
#define MINISYNI_H

#include <stdint.h>

/* We don't allocate memory, so no cleanup is necessary, and
 * failure is only possible for bad params.
 * Mono only.
 * Requires (rate>0).
 */
void minisyni_init(uint16_t rate);

/* Generate (c) samples beginning at (v).
 * Always *samples* regardless of channel count. Must be a multiple of (chanc).
 */
void minisyni_update(int16_t *v,int32_t c);

/* Stop the current song and begin a new one from the top.
 * (force) nonzero to restart the current song, if you've asked to play the thing currently playing.
 * Otherwise we keep it in its current position.
 * (0,0,0,0) for "no song".
 * See etc/doc/minisyni-song.txt for (v) format.
 * Returns:
 *   >0 changed song.
 *    0 success, retained current song.
 *   <0 error, retained current song.
 */
int8_t minisyni_play_song(const uint8_t *v,uint16_t c,uint8_t force,uint8_t repeat);

/* Stop all voices gently (release) or cold (silence).
 */
void minisyni_release_all();
void minisyni_silence_all();

//TODO configure voices

/* We accept all MIDI channel events but mostly ignore them.
 * Only note_on and note_off do anything -- I might implement pitch_wheel later.
 * A special "fire_forget" event plays an unaddressable note with a fixed time before release.
 */
void minisyni_fire_forget(uint8_t waveid,uint8_t noteid,uint16_t ttl/* frames */);
void minisyni_note_on(uint8_t chid,uint8_t noteid,uint8_t velocity);
void minisyni_note_off(uint8_t chid,uint8_t noteid,uint8_t velocity);
void minisyni_note_adjust(uint8_t chid,uint8_t noteid,uint8_t velocity);
void minisyni_control_change(uint8_t chid,uint8_t key,uint8_t value);
void minisyni_program_change(uint8_t chid,uint8_t pid);
void minisyni_channel_pressure(uint8_t chid,uint8_t pressure);
void minisyni_pitch_wheel(uint8_t chid,int16_t v/* -8192..8191 */);

#endif
