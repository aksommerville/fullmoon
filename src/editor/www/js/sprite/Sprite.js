/* Sprite.js
 */
 
export class Sprite {
  constructor(serial, path) {
    this.path = path;
    // Basically, we'll store everything as encoded.
    this.type = "dummy";
    this.image = "mainsprites";
    this.tileid = "0";
    this.xform = "0";
    this.flags = "0";
    this.layer = "0";
    this.unknownFields = {};
    if (serial) this._decode(serial);
  }
  
  encode() {
    let dst = "";
    for (const key of Sprite.FIELD_NAMES) {
      dst += `${key} ${this[key]}\n`;
    }
    for (const key of Object.keys(this.unknownFields)) {
      dst += `${key} ${this.unknownFields[key]}\n`;
    }
    return dst;
  }
  
  _decode(serial) {
    const src = new TextDecoder().decode(serial);
    let srcp = 0;
    while (srcp < src.length) {
      let nlp = src.indexOf("\n", srcp);
      if (nlp < 0) nlp = src.length;
      const [yadda, key, value] = src.substring(srcp, nlp).match(/^\s*([^\s]+)\s+(.*)$/);
      if (Sprite.FIELD_NAMES.includes(key)) {
        this[key] = value;
      } else {
        this.unknownFields[key] = value;
      }
      srcp = nlp + 1;
    }
  }
}

// These are members of Sprite instance, and also keys in the serial data.
// Every field is typed string, even the numeric ones.
Sprite.FIELD_NAMES = [
  "type", "image", "tileid", "xform", "flags", "layer",
];

Sprite.FIELD_COMMENTS = {
  type: "Omit the leading 'fmn_sprtype_'",
  image: "Stem of an 'image' resource",
  tileid: "0..255",
  xform: "0 XREV YREV SWAP",
  flags: "0 SOLID RAINABLE OPENABLE",
  layer: "-128..127",
};
