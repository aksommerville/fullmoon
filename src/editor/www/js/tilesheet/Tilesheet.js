/* Tilesheet.js
 * Model of a tilesheet resource, /res/image/*_props.txt
 * Defines various properties for each tile of a 256-tile image.
 */
 
export class Tilesheet {
  constructor(serial, path) {
    this.path = path;
    this.tiles = [];
    for (let i=0; i<256; i++) this.tiles.push({
      props: 0,
      group: 0,
      mask: 0,
      priority: 0,
    });
    if (serial) this._decode(serial);
  }
  
  encode() {
    let dst = "";
    for (const bank of ["props", "group", "mask", "priority"]) {
      for (let row=0,p=0; row<16; row++) {
        for (let col=0; col<16; col++,p++) {
          const n = this.tiles[p][bank];
          dst += (n >> 4).toString(16);
          dst += (n & 15).toString(16);
        }
        dst += "\n";
      }
      dst += "\n"; // blank line between banks for what laughably passes as legibility
    }
    return dst;
  }
  
  _decode(serial) {
    const src = new TextDecoder().decode(serial);
    let srcp = 0, lineno = 0;
    let y = 0;
    while (srcp < src.length) {
      lineno++;
      let nlp = src.indexOf('\n', srcp);
      if (nlp < 0) nlp = src.length;
      const line = src.substring(srcp, nlp).trim();
      srcp = nlp + 1;
      if (!line) continue;
      if (line.length !== 32) throw new Error(`${lineno}: Expected length 32, found ${line.length}`);
      
      let bank = "";
      switch (y >> 4) {
        case 0: bank = "props"; break;
        case 1: bank = "group"; break;
        case 2: bank = "mask"; break;
        case 3: bank = "priority"; break;
      }
      let bankp = ((y & 15) << 4);
      
      for (let x=0; x<16; x++,bankp++) {
        const v=parseInt(line.substr(x*2, 2), 16);
        if (isNaN(v)) throw new Error(`${lineno}: Invalid hex byte '${line.substr(x*2, 2)}'`);
        if (!bank) continue; // extra data, just validate syntax and discard it
        this.tiles[bankp][bank] = v;
      }
      y++;
    }
  }
  
  /* If you have (tileid) at a point in the map with neighbor mask (neighbors), what tileid ought it be?
   * (randomize) may cause us to shuffle, even if the current tileid is valid.
   */
  findBestFit(tileid, neighbors, randomize) {
    const group = this.tiles[tileid].group;
    if (!group) return tileid; // no group means no joining or randomization
    const fallbacks = [];
    const matches = [];
    for (let i=0; i<256; i++) {
      const tile = this.tiles[i];
      if (tile.group !== group) continue;
      if (tile.mask & ~neighbors) continue; // every bit in the tile must be present
      if (neighbors & ~tile.mask) fallbacks.push(i); // neighbors has extra bits, it's a fallback
      else matches.push(i); // perfect match for neighbors, that's a special thing
    }
    // If there are any perfect matches, that's all we'll consider.
    if (matches.length) {
      // They don't want shuffling, and the existing tile is a perfect match? Keep it.
      if (!randomize && matches.includes(tileid)) return tileid;
      return this.randomTile(matches);
    }
    // Same idea against fallbacks. But first reduce to only the highest count of matching neighbors.
    if (fallbacks.length) {
      const okids = [];
      let bestCount = -1;
      for (const tileid of fallbacks) {
        let match = this.tiles[tileid].mask & neighbors;
        let count = 0;
        for (; match; match>>=1) if (match & 1) count++;
        if (count > bestCount) {
          okids.splice(0, okids.length);
          bestCount = count;
          okids.push(tileid);
        } else if (count === bestCount) {
          okids.push(tileid);
        }
      }
      if (!randomize && okids.includes(tileid)) return tileid;
      return this.randomTile(okids);
    }
    // Finally, we got nothing so keep it as is.
    return tileid;
  }
  
  randomTile(tileids) {
    if (tileids.length <= 1) return tileids[0] || 0;
    // Add up all the candidate priorities.
    let range = 0;
    for (const tileid of tileids) range += this.tiles[tileid].priority;
    // If it's zero, then the whole set contains non-randomizable tiles, which is possible.
    // In that case, pretend they all have equal weight.
    if (!range) return tileids[Math.floor(Math.random() * tileids.length)];
    // Otherwise, weighted random such that zero-priority tiles will never be chosen.
    let choice = Math.random() * range;
    for (const tileid of tileids) {
      choice -= this.tiles[tileid].priority;
      if (choice < 0) return tileid;
    }
    // Oops.
    return tileids[0];
  }
}

Tilesheet.PROPS_SOLID = 0x01;
Tilesheet.PROPS_HOLE = 0x02;
Tilesheet.PROP_NAMES = [ // by bit index
  "SOLID",
  "HOLE",
  "0x04",
  "0x08",
  "0x10",
  "0x20",
  "0x40",
  "0x80",
];
