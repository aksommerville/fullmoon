/* fmn_proximity.h
 * Manages state for active proximity triggers.
 * A proximity trigger is a map attachment that gets called with the hero's distance, every frame, when it's visible.
 * They don't necessarily care about that distance. Might be using this facility only for "update every frame".
 */
 
#ifndef FMN_PROXIMITY_H
#define FMN_PROXIMITY_H

/* Drop all triggers.
 * This does fire the farewell for any registered triggers.
 */
void fmn_proximity_clear();

/* Add a trigger to the list.
 * It will be called for the first time at the next update.
 * (distance) is not guaranteed to be exact; I plan to use an efficient approximation instead of true distance.
 * We may have an internal limit on trigger count, and quietly reject adds beyond that.
 * So if you have any initialization/cleanup, wait for the first update to initialize.
 * Any trigger that receives an update at all will eventually receive the (distance==0xffff) farewell.
 */
void fmn_proximity_add(
  int16_t x,int16_t y,
  uint8_t q1,uint8_t q2,uint8_t q3,
  void (*cb)(uint16_t distance,uint8_t q1,uint8_t q2,uint8_t q3)
);

/* Call every trigger, with its distance to (x,y).
 */
void fmn_proximity_update(int16_t x,int16_t y);

#endif
