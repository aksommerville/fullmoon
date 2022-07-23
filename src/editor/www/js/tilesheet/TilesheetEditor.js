/* TilesheetEditor.js
 */
 
import { Dom } from "../util/Dom.js";
import { Tilesheet } from "./Tilesheet.js";
import { ResourceService } from "../service/ResourceService.js";
import { TileModal } from "./TileModal.js";

export class TilesheetEditor {
  static getDependencies() {
    return [HTMLElement, Dom, "discriminator", ResourceService];
  }
  constructor(element, dom, discriminator, resourceService) {
    this.element = element;
    this.dom = dom;
    this.discriminator = discriminator;
    this.resourceService = resourceService;
    
    this.tilesheet = null;
    this.image = null;
    
    this.neighborsImage = new Image();
    this.neighborsImage.src = "/img/neighbors.png";
    
    this.buildUi();
  }
  
  setup(tilesheet) {
    this.tilesheet = tilesheet;
    this.image = null;
    const imagePath = this.tilesheet.path.replace(/_props.txt$/, ".png");
    this.resourceService.getImage(imagePath).then((image) => {
      this.image = image;
      this.render();
    }).catch((error) => {
      console.error(`Failed to load tilesheet image '${imagePath}'`, error);
    });
    this.render();
  }
  
  buildUi() {
    this.element.innerHTML = "";
    
    const buttonsRow = this.dom.spawn(this.element, "DIV", ["buttonsRow"], { "on-change": () => this.render() });
    this.spawnButton(buttonsRow, "Image");
    this.spawnButton(buttonsRow, "Blotter");
    this.spawnButton(buttonsRow, "Grid");
    this.spawnButton(buttonsRow, "Props");
    this.spawnButton(buttonsRow, "Group");
    this.spawnButton(buttonsRow, "Mask");
    this.spawnButton(buttonsRow, "Priority");
    
    this.dom.spawn(this.element, "CANVAS", { "on-click": (e) => this.onClickCanvas(e) });
  }
  
  spawnButton(parent, label) {
    const id = `${this.discriminator}-${label}-checkbox`;
    this.dom.spawn(parent, "INPUT", { type: "checkbox", name: label, id, checked: 'checked' });
    this.dom.spawn(parent, "LABEL", label, { for: id });
  }
  
  readVisibilityFeatures() {
    return Array.from(this.element.querySelectorAll(".buttonsRow input:checked")).map(e => e.getAttribute("name"));
  }
  
  presentTileModal(tileid) {
    const wrapper = this.dom.spawnModal();
    const controller = this.dom.spawnController(wrapper, TileModal);
    controller.setup(this.tilesheet, this.image, tileid);
    controller.ondirty = (tileid) => this.onTileChanged(tileid);
  }
  
  onTileChanged(tileid) {
    this.render();
    this.resourceService.dirty(this.tilesheet.path, () => this.tilesheet.encode()).then(() => {
    }).catch((e) => {
      if (e !== "replaced") {
        console.error(e);
      }
    });
  }
  
  onClickCanvas(e) {
    const bounds = this.imageBounds();
    const mx = e.layerX - e.target.offsetLeft;
    const my = e.layerY - e.target.offsetTop;
    if (mx < bounds.x) return;
    if (my < bounds.y) return;
    if (mx >= bounds.x + bounds.w) return;
    if (my >= bounds.y + bounds.h) return;
    const tilesize = bounds.w >> 4;
    if (!tilesize) return;
    const tileid = Math.floor((my - bounds.y) / tilesize) * 16 + Math.floor((mx - bounds.x) / tilesize);
    if ((tileid < 0) || (tileid > 0xff)) return;
    this.presentTileModal(tileid);
  }
  
  imageBounds(canvas) {
    if (!canvas) canvas = this.element.querySelector("canvas");
    const scale = Math.max(Math.min(
      Math.floor(canvas.width / this.image.naturalWidth),
      Math.floor(canvas.height / this.image.naturalHeight),
    ), 1);
    const w = this.image.naturalWidth * scale;
    const h = this.image.naturalHeight * scale;
    const x = (canvas.width >> 1) - (w >> 1);
    const y = (canvas.height >> 1) - (h >> 1);
    return { x, y, w, h };
  }
  
  /* Render.
   ******************************************************************/
  
  render() {
    const features = this.readVisibilityFeatures();
    const canvas = this.element.querySelector("canvas");
    const canvasBox = canvas.getBoundingClientRect();
    canvas.width = canvasBox.width;
    canvas.height = canvasBox.height;
    const context = canvas.getContext("2d");
    context.imageSmoothingEnabled = false;
    
    context.fillStyle = "#888";
    context.fillRect(0, 0, canvas.width, canvas.height);
    if (!this.image) return;
    const dst = this.imageBounds(canvas);
    const tilesize = dst.w >> 4; // dst should always be square, and a multiple of 128.
    
    if (features.includes("Image")) {
      context.drawImage(this.image, dst.x, dst.y, dst.w, dst.h);
    }
    
    if (features.includes("Blotter")) {
      context.fillStyle = "#444";
      context.globalAlpha = 0.8;
      context.fillRect(0, 0, canvas.width, canvas.height);
      context.globalAlpha = 1.0;
    }
    
    if (features.includes("Grid")) {
      context.beginPath();
      for (let p=0,i=17; i-->0; p+=tilesize) {
        context.moveTo(dst.x + p, dst.y);
        context.lineTo(dst.x + p, dst.y + dst.h);
        context.moveTo(dst.x, dst.y + p);
        context.lineTo(dst.x + dst.w, dst.y + p);
      }
      context.strokeStyle = "#0f0";
      context.stroke();
    }
    
    if (features.includes("Props")) {
      const dotcolors = [
        "#840", // SOLID
        "#00f", // HOLE
        "#f00", // undefined
        "#0f0", // undefined
        "#ff0", // undefined
        "#f0f", // undefined
        "#0ff", // undefined
        "#088", // undefined
      ];
      const dotsize = tilesize >> 3;
      for (let tileid=0; tileid<256; tileid++) {
        const props = this.tilesheet.tiles[tileid].props;
        if (!props) continue;
        const dsty = dst.y + (tileid >> 4) * tilesize;
        let dstx = dst.x + (tileid & 15) * tilesize;
        for (let mask=1,ix=0; mask<0x100; mask<<=1,ix++) {
          if (!(props & mask)) continue;
          context.fillStyle = dotcolors[ix];
          context.fillRect(dstx, dsty, dotsize, dotsize);
          dstx += dotsize;
        }
      }
    }
    
    if (features.includes("Mask") && this.neighborsImage) {
      for (let tileid=0; tileid<256; tileid++) {
        const mask = this.tilesheet.tiles[tileid].mask;
        if (!mask) continue;
        const dstx = dst.x + ((tileid & 15) + 1) * tilesize - 16;
        const dsty = dst.y + ((tileid >> 4) + 1) * tilesize - 16;
        if (mask & 0x80) context.drawImage(this.neighborsImage, 0, 0, 5, 5, dstx+ 0, dsty+ 0, 5, 5);
        if (mask & 0x40) context.drawImage(this.neighborsImage, 5, 0, 5, 5, dstx+ 5, dsty+ 0, 5, 5);
        if (mask & 0x20) context.drawImage(this.neighborsImage,10, 0, 5, 5, dstx+10, dsty+ 0, 5, 5);
        if (mask & 0x10) context.drawImage(this.neighborsImage, 0, 5, 5, 5, dstx+ 0, dsty+ 5, 5, 5);
        if (mask & 0x08) context.drawImage(this.neighborsImage,10, 5, 5, 5, dstx+10, dsty+ 5, 5, 5);
        if (mask & 0x04) context.drawImage(this.neighborsImage, 0,10, 5, 5, dstx+ 0, dsty+10, 5, 5);
        if (mask & 0x02) context.drawImage(this.neighborsImage, 5,10, 5, 5, dstx+ 5, dsty+10, 5, 5);
        if (mask & 0x01) context.drawImage(this.neighborsImage,10,10, 5, 5, dstx+10, dsty+10, 5, 5);
      }
    }
    
    if (features.includes("Group")) {
      for (let tileid=0; tileid<256; tileid++) {
        const group = this.tilesheet.tiles[tileid].group;
        if (!group) continue;
        const dstx = dst.x + (tileid & 15) * tilesize;
        const dsty = dst.y + ((tileid >> 4) + 1) * tilesize;
        context.fillStyle = "#ff0";
        context.fillText(`G:${group}`, dstx, dsty);
      }
    }
    
    if (features.includes("Priority")) {
      for (let tileid=0; tileid<256; tileid++) {
        const priority = this.tilesheet.tiles[tileid].priority;
        if (!priority) continue;
        const dstx = dst.x + (tileid & 15) * tilesize;
        const dsty = dst.y + ((tileid >> 4) + 0.7) * tilesize;
        context.fillStyle = "#0f0";
        context.fillText(`P:${priority}`, dstx, dsty);
      }
    }
  }
}
