#ifndef IMGCVT_H
#define IMGCVT_H

#include "tool/common/cli.h"
#include "tool/common/serial.h"
#include "tool/common/fs.h"
#include "tool/common/png.h"
#include "game/fullmoon.h" /* for constants only */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

struct imgcvt {
  struct cli cli;
  char *srcpath;
  char *dstpath;
  void *src,*dst;
  int srcc,dstc;
  struct png_image png;
  struct fmn_image image;
};

#define IMGCVT ((struct imgcvt*)cli)

int imgcvt_fmn_from_png(struct imgcvt *imgcvt);
int imgcvt_encode_c(struct imgcvt *imgcvt);

int fmn_image_get_size(const struct fmn_image *image);

#endif
