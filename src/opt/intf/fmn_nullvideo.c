#include "intf.h"
#include <stdio.h>

/* Object definition.
 */
 
struct video_driver_nullvideo {
  struct video_driver hdr;
};

#define DRIVER ((struct video_driver_nullvideo*)driver)

/* Cleanup.
 */
 
static void _nullvideo_del(struct video_driver *driver) {
}

/* Init.
 */
 
static int _nullvideo_init(struct video_driver *driver) {
  driver->fullscreen=1;
  return 0;
}

/* Swap buffers.
 */
 
static int _nullvideo_swap(struct video_driver *driver,const void *fb) {
  // If we want to take screencaps or something, could do that from here.
  return 0;
}

/* Type definition.
 */
 
const struct video_driver_type video_driver_type_nullvideo={
  .name="nullvideo",
  .desc="Dummy video driver that quietly discards all frames.",
  .objlen=sizeof(struct video_driver_nullvideo),
  .del=_nullvideo_del,
  .init=_nullvideo_init,
  .swap=_nullvideo_swap,
};
