#include "serial.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>

/* Cleanup.
 */
 
void encoder_cleanup(struct encoder *encoder) {
  if (encoder->v) free(encoder->v);
}

/* Grow buffer.
 */
 
int encoder_require(struct encoder *encoder,int addc) {
  if (addc<1) return 0;
  if (encoder->c>INT_MAX-addc) return -1;
  int na=encoder->c+addc;
  if (na<=encoder->a) return 0;
  if (na<INT_MAX-256) na=(na+256)&~255;
  void *nv=realloc(encoder->v,na);
  if (!nv) return -1;
  encoder->v=nv;
  encoder->a=na;
  return 0;
}

/* Replace.
 */
 
int encoder_replace(struct encoder *encoder,int p,int c,const void *src,int srcc) {
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (((char*)src)[srcc]) srcc++; }
  if (c<0) c=encoder->c-p;
  if ((p<0)||(c<0)||(p>encoder->c-c)) return -1;
  if (c!=srcc) {
    if (encoder_require(encoder,srcc-c)<0) return -1;
    memmove(encoder->v+p+srcc,encoder->v+p+c,encoder->c-c-p);
    encoder->c+=srcc-c;
  }
  memcpy(encoder->v+p,src,srcc);
  return 0;
}

/* Append raw data.
 */
 
int encode_raw(struct encoder *encoder,const void *src,int srcc) {
  if (!src) return 0;
  if (srcc<0) { srcc=0; while (((char*)src)[srcc]) srcc++; }
  if (encoder_require(encoder,srcc)<0) return -1;
  memcpy(encoder->v+encoder->c,src,srcc);
  encoder->c+=srcc;
  return srcc;
}

/* Append formatted string.
 */
 
int encode_fmt(struct encoder *encoder,const char *fmt,...) {
  if (!fmt||!fmt[0]) return 0;
  while (1) {
    va_list vargs;
    va_start(vargs,fmt);
    int err=vsnprintf(encoder->v+encoder->c,encoder->a-encoder->c,fmt,vargs);
    if ((err<0)||(err>=INT_MAX)) return -1;
    if (encoder->c<encoder->a-err) {
      encoder->c+=err;
      return err;
    }
    if (encoder_require(encoder,err+1)<0) return -1;
  }
}
