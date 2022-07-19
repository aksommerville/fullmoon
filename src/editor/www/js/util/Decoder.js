export class Decoder {
  constructor(src) {
    if (src instanceof ArrayBuffer) {
      this.src = new Uint8Array(src);
      this.p = 0;
    } else if (src instanceof Uint8Array) {
      this.src = src;
      this.p = 0;
    } else if (src instanceof Decoder) {
      this.src = src.src;
      this.p = src.p;
    } else {
      throw new Error(`Inappropriate argument to Decoder`);
    }
    this.textDecoder = new TextDecoder();
  }
  
  remaining() {
    return this.src.length - this.p;
  }
  
  require(c) {
    if (this.p > this.src.length - c) {
      throw new Error(`Decoder at ${this.p}/${this.src.length} requires ${c} more bytes`);
    }
  }
  
  u8() {
    this.require(1);
    return this.src[this.p++];
  }
  
  u16be() {
    this.require(2);
    const n = (this.src[this.p] << 8) | this.src[this.p + 1];
    this.p += 2;
    return n;
  }
  
  u32be() {
    this.require(4);
    const n = (this.src[this.p] << 24) | (this.src[this.p + 1] << 16) | (this.src[this.p + 2] << 8) | this.src[this.p + 3];
    this.p += 4;
    return n;
  }
  
  fixstr(len) {
    this.require(len);
    const v = this.textDecoder.decode(this.src.slice(this.p, this.p + len));
    this.p += len;
    return v;
  }
  
  raw(len) {
    this.require(len);
    const v = this.src.slice(this.p, this.p + len);
    this.p += len;
    return v;
  }
  
  pascalString() {
    const len = this.u8();
    return this.fixstr(len);
  }
}
