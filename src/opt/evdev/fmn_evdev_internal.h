#ifndef FMN_EVDEV_INTERNAL_H
#define FMN_EVDEV_INTERNAL_H

#include <stdio.h>
#include "opt/intf/intf.h"
#include "po_evdev.h"

struct input_driver_evdev {
  struct input_driver hdr;
  struct po_evdev *evdev;
};

#define DRIVER ((struct input_driver_evdev*)driver)

#endif
