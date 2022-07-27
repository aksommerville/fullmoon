/* png.h
 * Minimal PNG decoder.
 */

#ifndef PNG_H
#define PNG_H

#include <stdint.h>

// Arbitrary sanity limit so we don't have to check too many things.
#define PNG_SIZE_LIMIT 4096

struct png_image {
  uint8_t *pixels;
  int w,h;
  int stride;
  uint8_t depth,colortype;
  uint8_t *plte,*trns;
  int pltec,trnsc;
};

void png_image_cleanup(struct png_image *image);

/* (image) must be zeroed initially.
 * It does require cleanup, even if we fail here.
 */
int png_image_decode(struct png_image *image,const void *src,int srcc);

/* Call (cb) for each pixel in the image, in LRTB order.
 * Converts to luma+alpha from anything.
 */
int png_image_iterate_ya88(
  const struct png_image *image,
  int (*cb)(uint8_t y,uint8_t a,void *userdata),
  void *userdata
);
int png_image_iterate_rgba8888(
  const struct png_image *image,
  int (*cb)(uint8_t r,uint8_t g,uint8_t b,uint8_t a,void *userdata),
  void *userdata
);

#endif
