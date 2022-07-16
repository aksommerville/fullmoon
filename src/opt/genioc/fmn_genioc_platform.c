#include "fmn_genioc_internal.h"

/* Init.
 * We probably don't need this; we initialize things on our own.
 */
 
void fmn_platform_init() {
}

/* Update.
 * Like init, probably no need.
 */
 
void fmn_platform_update() {
}

/* Receive framebuffer.
 */
 
void fmn_platform_send_framebuffer(const void *fb) {
  if (video_driver_swap(fmn_genioc.intf->video,fb)<0) {
    fprintf(stderr,"Terminating due to error swapping video frames.\n");
    fmn_genioc.terminate=1;
  }
}

/* Report input.
 */

uint16_t fmn_platform_read_input() {
  return 0;//TODO input via genioc
}
