/* WasmAdapter.js
 * Load, link, and communicate with the Wasm module.
 */
 
export class WasmAdapter {
  constructor() {
    this.instance = null;
    this.fbdirty = false;
    this.fb = null;
    this._input = 0;
    this.highscore = 0;
  }
  
  /* Download and instantiate.
   * Does not run any Wasm code.
   * Returns a Promise.
   */
  load(path) {
    const params = this._generateParams();
    return WebAssembly.instantiateStreaming(fetch(path), params).then((instance) => {
      this.instance = instance;
      return instance;
    });
  }
  
  /* Call setup() in Wasm code.
   */
  init() {
    this.instance.instance.exports.setup();
  }
  
  /* Call loop() in Wasm code, with the input state (see InputManager.js)
   */
  update(input) {
    this._input = input;
    this.instance.instance.exports.loop();
  }
  
  /* If we have received a framebuffer dirty notification from Wasm,
   * return it as a Uint8Array (72 * 5) and mark clean.
   * Otherwise return null.
   */
  getFramebufferIfDirty() {
    if (this.fbdirty) {
      this.fbdirty = false;
      return this.fb;
    }
    return null;
  }
  
  /* Private.
   ***********************************************************************/
   
  _generateParams() {
    return {
      env: {
        millis: () => Date.now(),
        micros: () => Date.now() * 1000,
        srand: () => {},
        rand: () => Math.floor(Math.random() * 2147483647),
        fmn_platform_read_input: () => this._input,
        fmn_platform_send_framebuffer: (p) => this._receiveFramebuffer(p),
        abort: (...args) => {},
      },
    };
  }
  
  _receiveFramebuffer(p) {
    if (typeof(p) !== "number") return;
    const buffer = this.instance.instance.exports.memory.buffer;
    const fblen = 72 * 5;
    if ((p < 0) || (p + fblen > buffer.byteLength)) return;
    this.fb = new Uint8Array(buffer, p, fblen);
    this.fbdirty = true;
  }
  
}
