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

struct spritecvt {
  struct cli cli;
  char *srcpath;
  char *dstpath;
  char *src,*dst;
  int srcc,dstc;
  
  // Content. Anything we read as an integer must be 'int' regardless of its output size.
  char *tname,*imagename;
  int tnamec,imagenamec;
  int tileid,xform;
  int flags;
  int layer;
};

#define MAPCVT ((struct spritecvt*)cli)

int spritecvt_fmn_from_text(struct spritecvt *spritecvt);
int spritecvt_encode_c(struct spritecvt *spritecvt);

#endif
