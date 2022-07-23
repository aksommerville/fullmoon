#ifndef MAPCVT_H
#define MAPCVT_H

#include "tool/common/cli.h"
#include "tool/common/serial.h"
#include "tool/common/fs.h"
#include "game/fullmoon.h" /* for constants only */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

struct tileprops {
  struct cli cli;
  char *srcpath;
  char *dstpath;
  char *src,*dst;
  int srcc,dstc;
  int y; // 0..64, how many significant rows have we read
  uint8_t props[256];
  uint8_t groups[256];
  uint8_t masks[256];
  uint8_t priorities[256];
};

#define MAPCVT ((struct tileprops*)cli)

int tileprops_fmn_from_text(struct tileprops *tileprops);
int tileprops_encode_c(struct tileprops *tileprops);

#endif
