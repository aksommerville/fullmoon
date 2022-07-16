#include "fmn_genioc_internal.h"
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/* Current real time.
 */
 
double fmn_genioc_now_s() {
  struct timeval tv={0};
  gettimeofday(&tv,0);
  return tv.tv_sec+tv.tv_usec/1000000.0;
}

int64_t fmn_genioc_now_us() {
  struct timeval tv={0};
  gettimeofday(&tv,0);
  return (int64_t)tv.tv_sec*1000000ll+tv.tv_usec;
}

/* Current CPU time.
 */
 
double fmn_genioc_now_cpu_s() {
  struct timespec tv={0};
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&tv);
  return tv.tv_sec+tv.tv_nsec/1000000000.0;
}

/* Sleep.
 */
 
void fmn_genioc_sleep_us(int us) {
  usleep(us);
}
