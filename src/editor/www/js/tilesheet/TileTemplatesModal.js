/* TileTemplatesModal.js
 * Right-click in tilesheet.
 * Presents options for assigning properties to multiple tiles at once, based on some predefined patterns.
 */
 
import { Dom } from "../util/Dom.js";
import { Tilesheet } from "./Tilesheet.js";

export class TileTemplatesModal {
  static getDependencies() {
    return [HTMLElement, Dom, "discriminator"];
  }
  constructor(element, dom, discriminator) {
    this.element = element;
    this.dom = dom;
    this.discriminator = discriminator;
    
    this.ondirty = () => {};
    
    this.tilesheet = null;
    this.image = null;
    this.tileid = 0;
    
    this.buildUi();
  }
  
  setup(tilesheet, image, tileid) {
    this.tilesheet = tilesheet;
    this.image = image;
    this.tileid = tileid;
    this.populateUi();
  }
  
  buildUi() {
    this.element.innerHTML = "";
    
    const propsRow = this.dom.spawn(this.element, "DIV", ["propsRow"]);
    for (let i=0; i<8; i++) this.addPropButton(propsRow, i);
    
    const menu = this.dom.spawn(this.element, "DIV", ["menu"]);
    for (const name of Object.keys(TileTemplatesModal.OPTIONS)) {
      const option = this.dom.spawn(menu, "BUTTON", ["option"], { "data-name": name, "on-click": (e) => this.onClick(e) });
      this.dom.spawn(option, "CANVAS");
      this.dom.spawn(option, "DIV", ["name"], name);
    }
  }
  
  addPropButton(parent, i) {
    const id = `${this.discriminator}-prop-${i}`;
    const input = this.dom.spawn(parent, "INPUT", ["prop"], { type: "checkbox", id, value: 1<<i });
    const label = this.dom.spawn(parent, "LABEL", { for: id }, Tilesheet.PROP_NAMES[i]);
  }
  
  populateUi() {
    const tile = this.tilesheet.tiles[this.tileid];
    for (const checkbox of this.element.querySelectorAll("input.prop")) {
      checkbox.checked = (+checkbox.value) & tile.props;
    }
    const col = this.tileid & 15;
    const row = this.tileid >> 4;
    for (const container of this.element.querySelectorAll(".menu .option")) {
      const name = container.getAttribute("data-name");
      const template = TileTemplatesModal.OPTIONS[name];
      if ((col + template.width > 16) || (row + template.height > 16)) {
        // Template would exceed image bounds. Don't offer it.
        container.classList.add("hidden");
      } else {
        const canvas = container.querySelector("canvas");
        this.drawPreview(canvas, name);
      }
    }
  }
  
  onClick(e) {
    let button = e.target;
    for (; button; button = button.parentNode) {
      if (button.tagName === "BUTTON") break;
    }
    if (!button) return;
    const name = button.getAttribute("data-name");
    const template = TileTemplatesModal.OPTIONS[name];
    if (!template) return;
    this.applyTemplate(template);
    this.dom.popModal();
  }
  
  drawPreview(canvas, name) {
    const template = TileTemplatesModal.OPTIONS[name];
    const scale = 2;
    const tileWidth = this.image.naturalWidth >> 4;
    const tileHeight = this.image.naturalHeight >> 4;
    canvas.width = tileWidth * template.width * scale;
    canvas.height = tileHeight * template.height * scale;
    const context = canvas.getContext("2d");
    context.imageSmoothingEnabled = false;
    const srcx = (this.tileid & 15) * tileWidth;
    const srcy = (this.tileid >> 4) * tileHeight;
    context.drawImage(this.image, srcx, srcy, tileWidth * template.width, tileHeight * template.height, 0, 0, canvas.width, canvas.height);
  }
  
  applyTemplate(template) {
    const group = this.tilesheet.unusedGroupId();
    const props = this.readPropsFromUi();
    for (let suby=0,tp=0; suby<template.height; suby++) {
      for (let subx=0; subx<template.width; subx++,tp++) {
        const tile = this.tilesheet.tiles[this.tileid + suby*16 + subx];
        if (!tile) continue;
        tile.props = props;
        tile.mask = template.masks[tp];
        tile.priority = template.priorities[tp];
        tile.group = group;
      }
    }
    this.ondirty();
  }
  
  readPropsFromUi() {
    let props = 0;
    for (const checkbox of this.element.querySelectorAll("input.prop:checked")) {
      props |= +checkbox.value;
    }
    return props;
  }
}

TileTemplatesModal.OPTIONS = {
  fatFill: {
    width: 5,
    height: 3,
    masks: [
      0x0b,0x1f,0x16,0xfe,0xfb,
      0x6b,0xff,0xd6,0xdf,0x7f,
      0x68,0xf8,0xd0,0x00,0x00,
    ],
    priorities: [
      255,255,255,255,255,
      255,255,255,255,255,
      255,255,255,200,100,
    ],
  },
  skinnyFill: {
    width: 4,
    height: 4,
    masks: [
      0x00,0x08,0x18,0x10,
      0x02,0x0a,0x1a,0x12,
      0x42,0x4a,0x5a,0x52,
      0x40,0x48,0x58,0x50,
    ],
    priorities: [
      255,255,255,255,
      255,255,255,255,
      255,255,255,255,
      255,255,255,255,
    ],
  },
  straightEight: {
    width: 8,
    height: 1,
    masks: [0,0,0,0,0,0,0,0],
    priorities: [255,225,195,165,135,105,75,45],
  },
  straightFour: {
    width: 4,
    height: 1,
    masks: [0,0,0,0],
    priorities: [255,200,150,100],
  },
};
