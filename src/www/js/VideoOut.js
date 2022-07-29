/* VideoOut.js
 * Manages the <canvas> and incoming framebuffers.
 */
 
import { Fullmoon } from "./Fullmoon.js"
 
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
    this.element.width = Fullmoon.FBW;
    this.element.height = Fullmoon.FBH;
    const scale = 6;
    this.element.style.width = `${72 * scale}px`;
    this.element.style.height = `${40 * scale}px`;
    this.element.style.imageRendering = 'crisp-edges';
    parent.appendChild(this.element);
    this.context = this.element.getContext("2d");
    this.imageData = this.context.createImageData(Fullmoon.FBW, Fullmoon.FBH);
  }
  
  /* Put the new image on screen.
   * (src) must be a Uint8Array of length FBW*FBH*4, straight off the Wasm app.
   */
  render(src) {
    this.imageData.data.set(src);
    this.context.putImageData(this.imageData, 0, 0);
  }
  
}
