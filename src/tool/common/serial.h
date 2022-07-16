/* serial.h
 */
 
#ifndef SERIAL_H
#define SERIAL_H

/* Primitive tokens.
 */
 
static inline int digit_eval(char src) {
  if ((src>='0')&&(src<='9')) return src-'0';
  if ((src>='a')&&(src<='z')) return src-'a'+10;
  if ((src>='A')&&(src<='Z')) return src-'A'+10;
  return -1;
}

int int_eval(int *dst,const char *src,int srcc);

/* Encoder.
 */
 
struct encoder {
  char *v;
  int c,a;
};

void encoder_cleanup(struct encoder *encoder);
int encoder_require(struct encoder *encoder,int addc);
int encoder_replace(struct encoder *encoder,int p,int c,const void *src,int srcc);

int encode_raw(struct encoder *encoder,const void *src,int srcc);
int encode_fmt(struct encoder *encoder,const char *fmt,...);

#endif
