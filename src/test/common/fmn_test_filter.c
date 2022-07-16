#include "test/fmn_test.h"
#include <string.h>

/* Filter test.
 */
 
int fmn_test_filter(const char *name,int argc,char **argv,int ignore,const char *tags) {
  //TODO compare name and tags against argv
  if (ignore) return 0;
  return 1;
}

/* Report test results.
 */
 
void fmn_report_test_result(int result,const char *name) {
  fprintf(stderr,"FMN_TEST %s %s\n",(result<0)?"FAIL":"PASS",name);
}

void fmn_report_test_skipped(const char *name) {
  fprintf(stderr,"FMN_TEST SKIP %s\n",name);
}
