#include <stdio.h>
#include <stdint.h>

FILE *const stderr=0;

void *memcpy(void *dst,const void *src,unsigned long c) {
  uint8_t *DST=dst;
  const uint8_t *SRC=src;
  for (;c-->0;DST++,SRC++) *DST=*SRC;
  return dst;
}

void *memmove(void *dst,const void *src,unsigned long c) {
  if (dst<src) return memcpy(dst,src,c);
  uint8_t *DST=dst; DST+=c-1;
  const uint8_t *SRC=src; SRC+=c-1;
  for (;c-->0;DST--,SRC--) *DST=*SRC;
  return dst;
}

int memcmp(const void *a,const void *b,unsigned long c) {
  const uint8_t *A=a,*B=b;
  int d;
  for (;c-->0;A++,B++) if (d=*B-*A) return d;
  return 0;
}

void *memset(void *dst,int src,unsigned long c) {
  uint8_t *DST=dst;
  for (;c-->0;DST++) *DST=src;
  return dst;
}

int fprintf(FILE *f,const char *fmt,...) {
  return 0;
}
