/* FullmoonMap.js
 */
 
export class FullmoonMap {
  constructor(serial, path) {
    this.path = path;
    this.w = 0;
    this.h = 0;
    this.tilesheet = ""; // name
    this.pois = []; // {x,y,q[4],qp}
    this.cells = [];
    if (serial) this._decode(serial);
  }
  
  resize(w, h) {
    if (
      !w || (w < 1) || (w > 255) || (w % 9) ||
      !h || (h < 1) || (h > 255) || (h % 5)
    ) throw new Error(`Invalid map dimensions ${w},${h}`);
    if ((w === this.w) && (h === this.h)) return;
    
    const nv = new Uint8Array(w * h);
    for (let y=Math.min(h, this.h); y-->0; ) {
      for (let x=Math.min(w, this.w); x-->0; ) {
        nv[y * w + x] = this.cells[y * this.w + x];
      }
    }
    
    for (const poi of this.pois) {
      if (poi.x >= w) poi.x = w - 1;
      if (poi.y >= h) poi.y = h - 1;
    }
    
    this.w = w;
    this.h = h;
    this.cells = nv;
  }
  
  setTile(x, y, tileid) {
    if ((x < 0) || (x >= this.w) || (y < 0) || (y >= this.h) || (tileid < 0) || (tileid >= 256)) {
      throw new Error(`FullmoonMap.setTile(${x},${y},${tileid}), size=${this.w},${this.h}`);
    }
    this.cells[y * this.w + x] = tileid;
  }
  
  encode() {
    let dst = "";
    dst += `tilesheet=${this.tilesheet}\n`;
    dst += `size=${this.w} ${this.h}\n`;
    for (const poi of this.pois) {
      dst += `poi=${poi.x} ${poi.y} ${FullmoonMap.POI_NAMES[poi.q[0]] || poi.q[0]} ${poi.q[1]} ${poi.q[2]} ${poi.q[3]} ${poi.qp}\n`;
    }
    dst += "BEGIN_BODY\n";
    for (let yi=this.h,rowp=0; yi-->0; rowp+=this.w) {
      for (let xi=this.w,colp=rowp; xi-->0; colp++) {
        dst += (this.cells[colp] >> 4).toString(16);
        dst += (this.cells[colp] & 15).toString(16);
      }
      dst += "\n";
    }
    return dst;
  }
  
  /* We're more tolerant than the specified format, to the extent that empty data is actually legal.
   * We'll validate on the way out. But being an editor, the models won't necessarily be valid at all times.
   */
  _decode(serial) {
    const src = new TextDecoder().decode(serial);
    let srcp = 0, lineno = 0;
    
    /* Header. */
    while (srcp < src.length) {
      lineno++;
      let nlp = src.indexOf('\n', srcp);
      if (nlp < 0) nlp = src.length;
      const line = src.substring(srcp, nlp);
      srcp = nlp + 1;
      if (!line) continue;
      if (line === "BEGIN_BODY") break;
      const [k, v] = line.split('=').map(s => s.trim());
      switch (k) {
        case "tilesheet": this.tilesheet = v; break;
        case "size": {
            const [w, h] = v.split(/\s+/).map(n => +n);
            this.resize(w, h);
          } break;
        case "poi": this._decodePoi(v, lineno); break;
        default: throw new Error(`${lineno}: Unknown map header key '${k}'`);
      }
    }
    
    /* Body. */
    let y = 0;
    while (srcp < src.length) {
      lineno++;
      let nlp = src.indexOf('\n', srcp);
      if (nlp < 0) nlp = src.length;
      const line = src.substring(srcp, nlp);
      srcp = nlp + 1;
      if (y >= this.w) throw new Error(`${lineno}: Too many rows, expected ${this.w}`);
      if (line.length !== this.w*2) throw new Error(`${lineno}: Expected length ${this.w*2}, found ${line.length}`);
      for (let x=0; x<this.w; x++) {
        const tileid=parseInt(line.substr(x*2, 2), 16);
        if (isNaN(tileid)) throw new Error(`${lineno}: Invalid tileid '${line.substr(x*2, 2)}'`);
        this.setTile(x, y, tileid);
      }
      y++;
    }
    if (y < this.h) throw new Error(`Expected ${this.h} rows, found ${y}`);	
  }
  
  _decodePoi(src, lineno) {
    const words = src.split(/\s+/);
    if (words.length < 3) throw new Error(`${lineno}: 'poi' must have at least (x,y,type)`);
    if (words.length > 7) throw new Error(`${lineno}: Limit 7 parameters to 'poi', found ${words.length}`);
    const poi = {
      x: +words[0],
      y: +words[1],
      q: [this._evalQ0(words[2]), +words[3] || 0, +words[4] || 0, +words[5] || 0],
      qp: words[6] || "",
    };
    if (
      isNaN(poi.x) || (poi.x < 0) || (poi.x > 255) ||
      isNaN(poi.y) || (poi.y < 0) || (poi.y > 255)
    ) throw new Error(`${lineno}: Invalid position (${words[0]},${words[1]})`);
    for (let i=0; i<4; i++) {
      if (isNaN(poi.q[i]) || (poi.q[i] < 0) || (poi.q[i] > 255)) {
        throw new Error(`${lineno}: poi.q[${i}] must be in 0..255, found '${words[2+i]}'`);
      }
    }
    this.pois.push(poi);
  }
  
  _evalQ0(src) {
    const p = FullmoonMap.POI_NAMES.indexOf(src.toUpperCase());
    if (p >= 0) return p;
    return +src;
  }
}

FullmoonMap.POI_NAMES = [
  "START",
  "DOOR",
  "SPRITE",
  "TREADLE",
  "VISIBILITY",
  "PROXIMITY",
  "EDGE_DOOR",
];
