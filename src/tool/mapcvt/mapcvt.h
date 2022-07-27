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

struct mapcvt {
  struct cli cli;
  char *srcpath;
  char *dstpath;
  char *src,*dst;
  int srcc,dstc;
  int progmem;
  
  /* (tilesheet,tileprops) are not used, must be null.
   * (poiv[].qp) if not null are strings to insert verbatim in the C text (eg name of a map or sprdef).
   * Those will be freed at cleanup.
   */
  struct fmn_map map;
  int mappoia;
  char *tilesheetname;
};

#define MAPCVT ((struct mapcvt*)cli)

int mapcvt_fmn_from_text(struct mapcvt *mapcvt);
int mapcvt_encode_c(struct mapcvt *mapcvt);

#endif
