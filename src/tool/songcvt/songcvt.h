#ifndef SONGCVT_H
#define SONGCVT_H

#include "tool/common/cli.h"
#include "tool/common/serial.h"
#include "tool/common/fs.h"
#include "game/fullmoon.h" /* for constants only */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

struct songcvt {
  struct cli cli;
  char *srcpath;
  char *dstpath;
  char *src,*dst;
  int srcc,dstc;
  int progmem;
  struct encoder bin; // contains the raw serial data. (dst) is the C code.
};

#define SONGCVT ((struct songcvt*)cli)

int songcvt_minisyni_from_midi(struct songcvt *songcvt);
int songcvt_encode_c(struct songcvt *songcvt);

#endif
