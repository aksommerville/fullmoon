/* fmn_image.h
 * Generic image processing.
 */
 
#ifndef FMN_IMAGE_H
#define FMN_IMAGE_H

/* Image.
 *************************************************************/

#define FMN_IMGFMT_thumby     1 /* The Thumby framebuffer. y1 where each byte is 8 rows (not columns!) */
#define FMN_IMGFMT_ya11       2 /* Big-endian 2-bit luma+alpha, for sprites. */
#define FMN_IMGFMT_bgr565be   3
#define FMN_IMGFMT_rgba8888   4
#define FMN_IMGFMT_y1         5 /* Ordinary big-endian y1. */
#define FMN_IMGFMT_y8         6
#define FMN_IMGFMT_bgr332     7
#define FMN_IMGFMT_argb4444be 8 /* Picosystem */

#define FMN_FOR_EACH_IMGFMT \
  _(thumby) \
  _(ya11) \
  _(bgr565be) \
  _(rgba8888) \
  _(y1) \
  _(y8) \
  _(bgr332) \
  _(argb4444be)

#define FMN_XFORM_XREV 1
#define FMN_XFORM_YREV 2
#define FMN_XFORM_SWAP 4

struct fmn_image {
  uint8_t *v;
  // NB size of (v) is not necessarily (h*stride); thumby format is different.
  int16_t w,h,stride;
  uint8_t fmt;
  uint8_t writeable;
  uint8_t alpha; // If relevant per (fmt), must be nonzero to enable alpha. eg "rgba" vs "rgbx"
};

void fmn_image_clear(struct fmn_image *dst);

void fmn_image_fill_rect(
  struct fmn_image *dst,
  int16_t x,int16_t y,int16_t w,int16_t h,
  uint32_t pixel
);

void fmn_blit(
  struct fmn_image *dst,int16_t dstx,int16_t dsty,
  const struct fmn_image *src,int16_t srcx,int16_t srcy,
  int16_t w,int16_t h,
  uint8_t xform
);

/* Iterator.
 ******************************************************************************/

struct fmn_image_iterator_1d {
  uint8_t *p;
  uint8_t q;
  int16_t stride;
  void (*next)(struct fmn_image_iterator_1d *iter);
};

struct fmn_image_iterator {
  uint32_t (*read)(const uint8_t *p,uint8_t q);
  void (*write)(uint8_t *p,uint8_t q,uint32_t pixel);
  struct fmn_image *image;
  int16_t minorc,minorc0,majorc;
  struct fmn_image_iterator_1d major,minor;
};

/* Request must be fully in bounds or we fail.
 * On failure, return zero and neuter (iter) -- it is still safe to use.
 * With FMN_XFORM_SWAP, we reverse (w,h). Be mindful of that when bounds-checking.
 * You must write pixels of the correct size. Small formats might overwrite neighbors if you have unexpected bits set.
 */
uint8_t fmn_image_iterate(
  struct fmn_image_iterator *iter,
  const struct fmn_image *image,
  int16_t x,int16_t y,int16_t w,int16_t h,
  uint8_t xform
);

uint8_t fmn_image_iterator_next(struct fmn_image_iterator *iter);

static inline uint32_t fmn_image_iterator_read(const struct fmn_image_iterator *iter) {
  return iter->read(iter->minor.p,iter->minor.q);
}

static inline void fmn_image_iterator_write(struct fmn_image_iterator *iter,uint32_t src) {
  iter->write(iter->minor.p,iter->minor.q,src);
}

/* Format analysis.
 ********************************************************************/

typedef uint32_t (*fmn_pixcvt_fn)(uint32_t src);
fmn_pixcvt_fn fmn_pixcvt_get(uint8_t dstfmt,uint8_t srcfmt);

// 0 means fully opaque always; -1 means any nonzero pixel is opaque; otherwise at least one bit of the mask must be set.
// We never blend during blits, it's all or nothing. (even if we add formats that in theory could, we won't).
uint32_t fmn_imgfmt_get_alpha_mask(uint8_t imgfmt);

#endif
