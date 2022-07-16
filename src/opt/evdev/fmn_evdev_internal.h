#ifndef FMN_EVDEV_INTERNAL_H
#define FMN_EVDEV_INTERNAL_H

//TODO evdev. this is a stub

#include "opt/intf/intf.h"

struct input_driver_evdev {
  struct input_driver hdr;
};

#define DRIVER ((struct input_driver_evdev*)driver)

#endif
