#include "fmn_evdev_internal.h"
  
/* Cleanup.
 */
 
static void _evdev_del(struct input_driver *driver) {
  po_evdev_del(DRIVER->evdev);
}

/* Callback from the inner driver.
 */
 
static int _evdev_po_cb(struct po_evdev *evdev,uint8_t btnid,int value) {
  struct input_driver *driver=po_evdev_get_userdata(evdev);
  if (driver->delegate.premapped_event) return driver->delegate.premapped_event(driver,btnid,value);
  return 0;
}

/* Init.
 */
 
static int _evdev_init(struct input_driver *driver) {
  if (!(DRIVER->evdev=po_evdev_new(_evdev_po_cb,driver))) return -1;
  return 0;
}

/* Update.
 */
 
static int _evdev_update(struct input_driver *driver) {
  return po_evdev_update(DRIVER->evdev);
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
};
