#include "serial.h"
#include <limits.h>

/* Evaluate integer.
 */

int int_eval(int *dst,const char *src,int srcc) {
  if (!src) return -1;
  if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  int srcp=0,positive=1,base=10;
  
  if (srcp>=srcc) return -1;
  if (src[srcp]=='-') {
    if (++srcp>=srcc) return -1;
    positive=0;
  } else if (src[srcp]=='+') {
    if (++srcp>=srcc) return -1;
  }
  
  if ((srcp<=srcc-3)&&(src[srcp]=='0')) switch (src[srcp+1]) {
    case 'b': case 'B': base= 2; srcp+=2; break;
    case 'o': case 'O': base= 8; srcp+=2; break;
    case 'd': case 'D': base=10; srcp+=2; break;
    case 'x': case 'X': base=16; srcp+=2; break;
  }
  
  int limit,overflow=0;
  if (positive) limit=UINT_MAX/base;
  else limit=INT_MIN/base;
  *dst=0;
  while (srcp<srcc) {
    int digit=digit_eval(src[srcp++]);
    if ((digit<0)||(digit>=base)) return -1;
    if (positive) {
      if ((unsigned int)(*dst)>limit) overflow=1;
      (*dst)=(unsigned int)(*dst)*base;
      if ((unsigned int)(*dst)>UINT_MAX-digit) overflow=1;
      (*dst)+=digit;
    } else {
      if (*dst<limit) overflow=1;
      (*dst)*=base;
      if (*dst<INT_MIN+digit) overflow=1;
      (*dst)-=digit;
    }
  }
  
  if (overflow) return 0;
  if (positive&&(*dst<0)) return 1;
  return 2;
}
