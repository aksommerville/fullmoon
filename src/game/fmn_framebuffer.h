/* fmn_framebuffer.h
 * Exports a bunch of macros detailing the global framebuffer format.
 *
 * At compile you should define exactly one of these nonzero:
 *  - FMN_IMAGE_SET_24c
 *  - FMN_IMAGE_SET_8b
 *  - FMN_IMAGE_SET_8c
 * And if 24c, also define the framebuffer format:
 *  - FMN_FRAMEBUFFER_bgr565be
 *  - FMN_FRAMEBUFFER_argb4444be
 *  - FMN_FRAMEBUFFER_rgba8888 (default)
 * If none is set, we assume 8c/bgr332, which is the format for Tiny.
 * (Our build system uses arduino-builder for Tiny and it's complicated to defined symbols, that's why it's the default).
 *
 * Some gotchas:
 *  - FMN_MM_PER_TILE is not necessarily a multiple of FMN_TILESIZE. There is no "FMN_MM_PER_PIXEL".
 */
 
#ifndef FMN_FRAMEBUFFER_H
#define FMN_FRAMEBUFFER_H

#if FMN_IMAGE_SET_24c
  #define FMN_GFXSCALE 3
  #define FMN_FBW (72*FMN_GFXSCALE)
  #define FMN_FBH (40*FMN_GFXSCALE)
  #define FMN_TILESIZE 24
  #if FMN_FRAMEBUFFER_bgr565be
    #define FMN_FBFMT FMN_IMGFMT_bgr565be
    #define FMN_FB_STRIDE (FMN_FBW*2)
    #define FMN_FB_SIZE_BYTES (FMN_FBW*FMN_FBH*2)
  #elif FMN_FRAMEBUFFER_argb4444be
    #define FMN_FBFMT FMN_IMGFMT_argb4444be
    #define FMN_FB_STRIDE (FMN_FBW*2)
    #define FMN_FB_SIZE_BYTES (FMN_FBW*FMN_FBH*2)
  #else /* rgba8888 */
    #define FMN_FBFMT FMN_IMGFMT_rgba8888
    #define FMN_FB_STRIDE (FMN_FBW*4)
    #define FMN_FB_SIZE_BYTES (FMN_FBW*FMN_FBH*4)
  #endif
  
#elif FMN_IMAGE_SET_8b
  #define FMN_FBW 72
  #define FMN_FBH 40
  #define FMN_FBFMT FMN_IMGFMT_thumby
  #define FMN_FB_STRIDE FMN_FBW
  #define FMN_FB_SIZE_BYTES ((FMN_FBW*FMN_FBH)>>3)
  #define FMN_TILESIZE 8
  #define FMN_GFXSCALE 1
  
#else /* 8c */
  #define FMN_FBW 72
  #define FMN_FBH 40
  #define FMN_FBFMT FMN_IMGFMT_bgr332
  #define FMN_FB_STRIDE FMN_FBW
  #define FMN_FB_SIZE_BYTES (FMN_FBW*FMN_FBH)
  #define FMN_TILESIZE 8
  #define FMN_GFXSCALE 1
#endif

/* Normalized screen coordinates.
 * So we can write layout code assuming a 72x40 framebuffer and have it scale up as needed.
 */
#define FMN_NSCOORD(x,y) (x)*FMN_GFXSCALE,(y)*FMN_GFXSCALE

#define FMN_SCREENW_TILES 9
#define FMN_SCREENH_TILES 5
#define FMN_MM_PER_TILE 64 /* <128, and power of two probably a good idea */
#define FMN_SCREENW_MM (FMN_SCREENW_TILES*FMN_MM_PER_TILE)
#define FMN_SCREENH_MM (FMN_SCREENH_TILES*FMN_MM_PER_TILE)

#endif
