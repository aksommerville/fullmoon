#ifndef FULLMOON_H
#define FULLMOON_H

#include <stdint.h>
#include <stdio.h>

/* Game implements these entry points.
 * Keep the signatures like this, in case we want to port to Arduino.
 */
void setup();
void loop();

/* The low 6 buttons are all we have on Thumby.
 * Could add others for features only available on higher platforms, eg fullscreen toggle, quit...
 */
#define FMN_BUTTON_LEFT    0x0001
#define FMN_BUTTON_RIGHT   0x0002
#define FMN_BUTTON_UP      0x0004
#define FMN_BUTTON_DOWN    0x0008
#define FMN_BUTTON_A       0x0010
#define FMN_BUTTON_B       0x0020

#endif
