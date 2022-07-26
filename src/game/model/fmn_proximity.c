#include "game/fullmoon.h"
#include "game/model/fmn_proximity.h"

// Limit for triggers active at one time. (this is per-screen, not per-map)
#define FMN_PRX_TRIGGER_LIMIT 32

/* Globals.
 */
 
static struct fmn_prx {
  struct fmn_prx_trigger {
    int16_t x;
    int16_t y;
    uint8_t q1,q2,q3;
    void (*cb)(uint16_t distance,uint8_t q1,uint8_t q2,uint8_t q3);
  } triggerv[FMN_PRX_TRIGGER_LIMIT];
  uint8_t triggerc;
} fmn_prx={0};

/* Clear triggers.
 */

void fmn_proximity_clear() {
  struct fmn_prx_trigger *trigger=fmn_prx.triggerv;
  uint8_t i=fmn_prx.triggerc;
  fmn_prx.triggerc=0;
  for (;i-->0;trigger++) {
    trigger->cb(0xffff,trigger->q1,trigger->q2,trigger->q3);
  }
}

/* Add trigger.
 */

void fmn_proximity_add(
  int16_t x,int16_t y,
  uint8_t q1,uint8_t q2,uint8_t q3,
  void (*cb)(uint16_t distance,uint8_t q1,uint8_t q2,uint8_t q3)
) {
  if (!cb) return;
  if (fmn_prx.triggerc>=FMN_PRX_TRIGGER_LIMIT) return;
  struct fmn_prx_trigger *trigger=fmn_prx.triggerv+fmn_prx.triggerc++;
  trigger->x=x;
  trigger->y=y;
  trigger->q1=q1;
  trigger->q2=q2;
  trigger->q3=q3;
  trigger->cb=cb;
}

/* Calculate distance.
 */
 
static inline uint16_t fmn_proximity_calculate_distance(int16_t ax,int16_t ay,int16_t bx,int16_t by) {

  // Absolute value of axiswise distances, and make (dx) the larger one.
  int16_t dx=ax-bx; if (dx<0) dx=-dx;
  int16_t dy=ay-by; if (dy<0) dy=-dy;
  if (dy>dx) { int16_t tmp=dx; dx=dy; dy=tmp; }
  
  // See src/test/unit/experiment/test_distance.c; maximum error about 12%
  return dx+(dy>>1);
}

/* Update.
 */

void fmn_proximity_update(int16_t x,int16_t y) {
  struct fmn_prx_trigger *trigger=fmn_prx.triggerv;
  uint8_t i=fmn_prx.triggerc;
  for (;i-->0;trigger++) {
    trigger->cb(
      fmn_proximity_calculate_distance(x,y,trigger->x,trigger->y),
      trigger->q1,trigger->q2,trigger->q3
    );
  }
}
