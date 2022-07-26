/* VideoOut.js
 * Manages the <canvas> and incoming framebuffers.
 */
 
export class VideoOut {
  constructor() {
    this.element = null;
    this.context = null;
    this.imageData = null;
  }
  
  /* Create my <canvas> element and attach to (parent).
   */
  setup(parent) {
    this.element = document.createElement("CANVAS");
    this.element.width = 72;
    this.element.height = 40;
    const scale = 6;
    this.element.style.width = `${72 * scale}px`;
    this.element.style.height = `${40 * scale}px`;
    this.element.style.imageRendering = 'crisp-edges';
    parent.appendChild(this.element);
    this.context = this.element.getContext("2d");
    this.imageData = this.context.createImageData(72, 40);
  }
  
  /* Put the new image on screen.
   * (src) must be a Uint8Array of length 72 * 5, straight off the Wasm app.
   * This part is enormously expensive (and it's a ridiculously small framebuffer too).
   * Between the two things (copying out framebuffer, then putImageData), it burns like 50% CPU on my box.
   * If we're going to do WebAssembly games going forward, don't ever do bulk framebuffer transfer!
   */
  render(src) {
    let srcrowp = 0, srcmask=0x01, dstp = 0, i = 72 * 40;
    for (let yi=40; yi-->0; ) {
      for (let xi=72, srcp=srcrowp; xi-->0; dstp+=4, srcp++) {
        if (src[srcp] & srcmask) {
          this.imageData.data[dstp + 0] = 0xff;
          this.imageData.data[dstp + 1] = 0xff;
          this.imageData.data[dstp + 2] = 0xff;
          this.imageData.data[dstp + 3] = 0xff;
        } else {
          this.imageData.data[dstp + 0] = 0x00;
          this.imageData.data[dstp + 1] = 0x00;
          this.imageData.data[dstp + 2] = 0x00;
          this.imageData.data[dstp + 3] = 0xff;
        }
      }
      if (srcmask === 0x80) {
        srcmask = 0x01;
        srcrowp += 72;
      } else {
        srcmask <<= 1;
      }
    }
    this.context.putImageData(this.imageData, 0, 0);
  }
  
}
