/* fmn_machid_driver.c
 * Glue between fmn_machid and our 'intf' unit.
 * This only does Fullmoon-specific stuff; if you're copying to some new project you only need fmn_machid.[ch]
 */
 
#include "fmn_machid.h"
#include "opt/intf/intf.h"

/* Object definition.
 */

struct input_driver_machid {
  struct input_driver hdr;
};

#define DRIVER ((struct input_driver_machid*)driver)

//TODO

/* Cleanup.
 */

static void _machid_del(struct input_driver *driver) {
}

/* Init.
 */

static int _machid_init(struct input_driver *driver) {
  return 0;
}

/* Update.
 */

static int _machid_update(struct input_driver *driver) {
  return 0;
}

/* Get device IDs.
 */

static const char *_machid_device_get_ids(int *vid,int *pid,struct input_driver *driver,int devid) {
  return 0;
}

/* Iterate device buttons.
 */

static int _machid_device_iterate(
  struct input_driver *driver,
  int devid,
  int (*cb)(struct input_driver *driver,int devid,int btnid,int hidusage,int value,int lo,int hi,void *userdata),
  void *userdata
) {
  return 0;
}

/* Drop device.
 */

static int _machid_device_drop(struct input_driver *driver,int devid) {
  return 0;
}

/* Type definition.
 */

const struct input_driver_type input_driver_type_machid={
  .name="machid",
  .desc="USB-HID for MacOS",
  .objlen=sizeof(struct input_driver_machid),
  .del=_machid_del,
  .init=_machid_init,
  .update=_machid_update,
  .device_get_ids=_machid_device_get_ids,
  .device_iterate=_machid_device_iterate,
  .device_drop=_machid_device_drop,
};
