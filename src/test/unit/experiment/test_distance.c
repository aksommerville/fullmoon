#include "test/fmn_test.h"
#include <math.h>

/* I need to calculate 2d distances fast, and they don't need to be perfect.
 * Is it reasonable to take (dmax+dmin/2)? My gut says that's close.
 * ...conclusion: Yes this is a fair approximation.
 * Maximum error seems to be about 12%, when dmax==dmin*2, and tapers down towards the limits.
 * Better than the 28% error of Manhattan distance... I can live with this.
 */
 
static int approximate_2d_distance() {
  const int log=0;
  #define _(dmax,dmin) { \
    int exact=lround(sqrt(dmax*dmax+dmin*dmin)); \
    int approx=dmax+(dmin>>1); \
    double error=(double)approx/(double)exact; \
    FMN_ASSERT(error>=1.0,"error=%f exact=%d approx=%d dmax=%d dmin=%d",error,exact,approx,dmax,dmin) \
    if (log) fprintf(stderr,"(%d,%d): exact=%d approx=%d error=%f\n",dmax,dmin,exact,approx,error); \
  }
  _(1,1)
  _(1000,100)
  _(100,1)
  _(100,10)
  _(100,20)
  _(100,30)
  _(100,40)
  _(100,50)
  _(100,60)
  _(100,70)
  _(100,80)
  _(100,90)
  _(100,100)
  #undef _
  return 0;
}

/* TOC.
 */
 
int main(int argc,char **argv) {
  FMN_UTEST(approximate_2d_distance)
  return 0;
}
