/* TileModal.js
 */
 
import { Dom } from "../util/Dom.js";
import { Tilesheet } from "./Tilesheet.js";

export class TileModal {
  static getDependencies() {
    return [HTMLElement, Dom, "discriminator"];
  }
  constructor(element, dom, discriminator) {
    this.element = element;
    this.dom = dom;
    this.discriminator = discriminator;
    
    // Owner should set directly:
    this.ondirty = (tileid) => {};
    
    this.tilesheet = null;
    this.image = null;
    this.tileid = 0;
    
    this.element.addEventListener("change", (event) => this.onChange(event));
    
    this.buildUi();
  }
  
  /* We will modify (tilesheet) in place, and call (ondirty) every time.
   * We tell (ondirty) which tile changed, and it's not necessarily the one we started with.
   */
  setup(tilesheet, image, tileid) {
    this.tilesheet = tilesheet;
    this.image = image;
    this.tileid = tileid;
    this.populateUi();
  }
  
  buildUi() {
    this.element.innerHTML = "";
    
    const idRow = this.dom.spawn(this.element, "DIV", ["idRow"]);
    const navButtons = this.dom.spawn(idRow, "DIV", ["navButtons"]);
    this.dom.spawn(navButtons, "INPUT", { type: "button", value: "<", "on-click": () => this.onNav(-1, 0)});
    const vertNavColumn = this.dom.spawn(navButtons, "DIV", ["vertNavColumn"]);
    this.dom.spawn(vertNavColumn, "INPUT", { type: "button", value: "^", "on-click": () => this.onNav(0, -1)});
    this.dom.spawn(vertNavColumn, "INPUT", { type: "button", value: "v", "on-click": () => this.onNav(0, 1)});
    this.dom.spawn(navButtons, "INPUT", { type: "button", value: ">", "on-click": () => this.onNav(1, 0)});
    this.dom.spawn(idRow, "CANVAS", ["thumbnail"]);
    this.dom.spawn(idRow, "DIV", ["description"]);
    
    this.dom.spawn(this.element, "HR");
    
    const table = this.dom.spawn(this.element, "TABLE");
    const headerRow = this.dom.spawn(table, "TR");
    this.dom.spawn(headerRow, "TH", "Props");
    this.dom.spawn(headerRow, "TH", "Mask");
    this.dom.spawn(headerRow, "TH", "Addl");
    const dataRow = this.dom.spawn(table, "TR");
    this.buildPropsCell(this.dom.spawn(dataRow, "TD"));
    this.buildMaskCell(this.dom.spawn(dataRow, "TD"));
    this.buildAddlCell(this.dom.spawn(dataRow, "TD"));
  }
  
  buildPropsCell(parent) {
    const list = this.dom.spawn(parent, "UL", ["props"]);
    for (let i=0; i<8; i++) {
      const id = `${this.discriminator}-prop-${i}`;
      const li = this.dom.spawn(list, "LI");
      this.dom.spawn(list, "INPUT", { type: "checkbox", id, value: (1 << i) });
      this.dom.spawn(list, "LABEL", { for: id }, Tilesheet.PROP_NAMES[i]);
    }
  }
  
  buildMaskCell(parent) {
    const table = this.dom.spawn(parent, "TABLE", ["mask"]);
    for (let y=0,mask=0x80; y<3; y++) {
      const tr = this.dom.spawn(table, "TR");
      for (let x=0; x<3; x++) {
        const td = this.dom.spawn(tr, "TD");
        if ((x === 1) && (y === 1)) continue; // no middle content, also don't shift mask here
        this.dom.spawn(td, "INPUT", { type: "checkbox", value: mask });
        mask >>= 1;
      }
    }
  }
  
  buildAddlCell(parent) {
    const table = this.dom.spawn(parent, "TABLE");
    let tr = this.dom.spawn(table, "TR");
    this.dom.spawn(tr, "TD", "Group");
    let td = this.dom.spawn(tr, "TD");
    this.dom.spawn(td, "INPUT", { type: "number", min: 0, max: 255, name: "group" });
    tr = this.dom.spawn(table, "TR");
    this.dom.spawn(tr, "TD", "Priority");
    td = this.dom.spawn(tr, "TD");
    this.dom.spawn(td, "INPUT", { type: "number", min: 0, max: 255, name: "priority" });
  }
  
  populateUi() {
    this.element.querySelector(".idRow .description").innerText = this.generateDescription();
    this.drawThumbnail();
    const tile = this.tilesheet ? this.tilesheet.tiles[this.tileid] : { props: 0, group: 0, mask: 0, priority: 0 };
    for (let bit=1; bit<0x100; bit<<=1) {
      this.element.querySelector(`.props input[value='${bit}']`).checked = !!(tile.props & bit);
      this.element.querySelector(`.mask input[value='${bit}']`).checked = !!(tile.mask & bit);
    }
    this.element.querySelector("input[name='group']").value = tile.group;
    this.element.querySelector("input[name='priority']").value = tile.priority;
  }
  
  readModelFromUi() {
    const tile = { props: 0, group: 0, mask: 0, priority: 0 };
    for (let bit=1; bit<0x100; bit<<=1) {
      if (this.element.querySelector(`.props input[value='${bit}']`).checked) tile.props |= bit;
      if (this.element.querySelector(`.mask input[value='${bit}']`).checked) tile.mask |= bit;
    }
    tile.group = +this.element.querySelector("input[name='group']").value;
    tile.priority = +this.element.querySelector("input[name='priority']").value;
    return tile;
  }
  
  tileModelsEquivalent(a, b) {
    if (a.props !== b.props) return false;
    if (a.group !== b.group) return false;
    if (a.mask !== b.mask) return false;
    if (a.priority !== b.priority) return false;
    return true;
  }
  
  generateDescription() {
    if (!this.tilesheet) return "";
    const name = this.tilesheet.path.replace(/^.*\/([^\/.-]*).*$/, "$1");
    const v = (this.tileid >> 4).toString(16) + (this.tileid & 15).toString(16);
    return `${name} 0x${v}`;
  }
  
  drawThumbnail() {
    if (!this.image) return;
    const canvas = this.element.querySelector(".idRow .thumbnail");
    const tilesize = Math.max(1, Math.min(
      this.image.naturalWidth >> 4,
      this.image.naturalHeight >> 4
    ));
    canvas.width = tilesize;
    canvas.height = tilesize;
    const context = canvas.getContext("2d");
    const col = this.tileid & 15;
    const row = this.tileid >> 4;
    context.drawImage(this.image, col * tilesize, row * tilesize, tilesize, tilesize, 0, 0, tilesize, tilesize);
  }
  
  onNav(dx, dy) {
    let col = (this.tileid & 15) + dx;
    if (col < 0) col = 15;
    else if (col >= 16) col = 0;
    let row = (this.tileid >> 4) + dy;
    if (row < 0) row = 15;
    else if (row >= 16) row = 0;
    const newTileid = (row << 4) | col;
    this.setup(this.tilesheet, this.image, newTileid);
  }
  
  onChange(event) {
    if (!this.tilesheet) return;
    const prev = this.tilesheet.tiles[this.tileid];
    const next = this.readModelFromUi();
    if (this.tileModelsEquivalent(prev, next)) return;
    this.tilesheet.tiles[this.tileid] = next;
    this.ondirty(this.tileid);
  }
  
}
