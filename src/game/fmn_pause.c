#include "fullmoon.h"
#include "fmn_pause.h"
#include "fmn_data.h"

/* Globals.
 */
 
static uint8_t fbdirty=0;

/* Begin.
 */
 
void fmn_pause_begin() {
  fbdirty=1;
}

/* End.
 */
 
void fmn_pause_end() {
}

/* Input.
 */
 
void fmn_pause_input(uint16_t input,uint16_t prev) {
  fbdirty=1;
}

/* Update.
 */
 
void fmn_pause_update() {
}

/* Render.
 */
 
void fmn_pause_render(struct fmn_image *fb) {
  if (!fbdirty) return;
  fbdirty=0;
}
