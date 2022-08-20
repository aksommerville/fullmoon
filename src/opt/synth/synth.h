/* synth.h
 * Interface to the synthesizer.
 * We build this for both native and tiny.
 */
 
#ifndef SYNTH_H
#define SYNTH_H

#include <stdint.h>

void synth_init(uint32_t rate);

void synth_update(int16_t *v,int c);

void synth_reset();
void synth_silence();
void synth_release_all();

void synth_event_realtime(uint8_t e);
void synth_event_note_off(uint8_t chid,uint8_t noteid,uint8_t velocity);
void *synth_event_note_on(uint8_t chid,uint8_t noteid,uint8_t velocity); // return value, don't worry about it
void synth_event_note_adjust(uint8_t chid,uint8_t noteid,uint8_t velocity);
void synth_event_control(uint8_t chid,uint8_t k,uint8_t v);
void synth_event_program(uint8_t chid,uint8_t pid);
void synth_event_pressure(uint8_t chid,uint8_t v);
void synth_event_wheel(uint8_t chid,uint16_t v);
void synth_event_fireforget(uint8_t chid,uint8_t noteid,uint32_t ttl);

int8_t synth_play_song(const void *src,uint16_t srcc,uint8_t force,uint8_t repeat);

extern const uint8_t song_sevencircles[];
extern const uint16_t song_sevencircles_length;

#endif
