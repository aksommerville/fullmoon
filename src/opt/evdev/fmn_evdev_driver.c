#include "fmn_evdev_internal.h"
  
/* Cleanup.
 */
 
static void _evdev_del(struct input_driver *driver) {
}

/* Init.
 */
 
static int _evdev_init(struct input_driver *driver) {
  return 0;
}

/* Update.
 */
 
static int _evdev_update(struct input_driver *driver) {
  return 0;
}

/* Get device IDs.
 */
 
static const char *_evdev_device_get_ids(int *vid,int *pid,struct input_driver *driver,int devid) {
  return 0;
}

/* Iterate device buttons.
 */
 
static int _evdev_device_iterate(
  struct input_driver *driver,
  int devid,
  int (*cb)(struct input_driver *driver,int devid,int btnid,int hidusage,int value,int lo,int hi,void *userdata),
  void *userdata
) {
  return 0;
}

/* Drop device.
 */
 
static int _evdev_device_drop(struct input_driver *driver,int devid) {
  return 0;
}

/* Type definition.
 */
 
const struct input_driver_type input_driver_type_evdev={
  .name="evdev",
  .desc="General input for Linux.",
  .objlen=sizeof(struct input_driver_evdev),
  .del=_evdev_del,
  .init=_evdev_init,
  .update=_evdev_update,
  .device_get_ids=_evdev_device_get_ids,
  .device_iterate=_evdev_device_iterate,
  .device_drop=_evdev_device_drop,
};
