/* MapToolbox.js
 * Appears in the left sidebar when a map is being edited.
 */
 
import { Dom } from "../util/Dom.js";
import { FullmoonMap } from "./FullmoonMap.js";
import { MapService } from "./MapService.js";

export class MapToolbox {
  static getDependencies() {
    return [HTMLElement, Dom, MapService, Window];
  }
  constructor(element, dom, mapService, window) {
    this.element = element;
    this.dom = dom;
    this.mapService = mapService;
    this.window = window;
    
    this.map = null;
    this.mapSubscription = this.mapService.subscribe(e => this.onMapEvent(e));
    this.tilesheetImage = this.mapService.tilesheetImage;
    this.paletteChoice = this.mapService.paletteChoice;
    this.leftTool = this.mapService.leftTool;
    this.rightTool = this.mapService.rightTool;
    
    this.buildUi();
  }
  
  onRemoveFromDom() {
    this.mapService.unsubscribe(this.mapSubscription);
  }
  
  setup(map) {
    this.map = map;
    this.element.querySelector("button.tilesheet").innerText = `Tilesheet: ${map.tilesheet}`;
    this.element.querySelector("button.resize").innerText = `Size: ${map.w},${map.h}`;
  }
  
  buildUi() {
    this.element.innerHTML = "";
    
    this.dom.spawn(this.element, "CANVAS", ["palette"], { width: 32, height: 32, "on-click": () => this.onEditPalette() });
    
    /* Interestingly, though, there is actually no need to click on a tool.
     * All 7 tools are reachable with some combination of left/right button and shift/control, by default.
     */
    const toolsRow = this.dom.spawn(this.element, "DIV", ["toolsRow"], {
      "on-click": (event) => this.onToolClick(event),
      "on-contextmenu": (event) => this.onToolClick(event), 
    });
    for (const name of MapService.TOOLS) {
      const button = this.dom.spawn(toolsRow, "IMG", ["tool"], { src: `/img/tool-${name}.png`, "data-name": name });
      if (name === this.leftTool) button.classList.add("selected-left");
      if (name === this.rightTool) button.classList.add("selected-right");
    }
    
    this.dom.spawn(this.element, "BUTTON", "Tilesheet:", ["tilesheet"], { "on-click": () => this.onClickTilesheet() });
    this.dom.spawn(this.element, "BUTTON", "Size:", ["resize"], { "on-click": () => this.onClickSize() });
  }
  
  onClickTilesheet() {
    if (!this.map) return;
    const newName = this.window.prompt("Tilesheet", this.map.tilesheet);
    if (!newName || (newName === this.map.tilesheet)) return;
    this.mapService.setTilesheetName(newName);
    this.element.querySelector("button.tilesheet").innerText = `Tilesheet: ${newName}`;
  }
  
  onClickSize() {
    if (!this.map) return;
    const newSize = this.window.prompt("Size (w,h), multiple of (9,5):", `${this.map.w},${this.map.h}`);
    if (!newSize) return;
    const [w, h] = newSize.split(',').map(n => +n.trim());
    this.mapService.resize(w, h);
  }
  
  onToolClick(event) {
    event.stopPropagation();
    event.preventDefault();
    if (!event.target.classList.contains("tool")) return;
    const name = event.target.getAttribute("data-name");
    if (!name) return;
    switch (event.button) {
      case 0: this.mapService.setLeftTool(name); break;
      case 2: this.mapService.setRightTool(name); break;
    }
    // don't react any further; let MapService digest and tell us it changed.
  }
  
  refreshToolHighlight() {
    for (const element of this.element.querySelectorAll(".tool.selected-left")) element.classList.remove("selected-left");
    for (const element of this.element.querySelectorAll(".tool.selected-right")) element.classList.remove("selected-right");
    this.element.querySelector(`.tool[data-name='${this.leftTool}']`).classList.add("selected-left");
    this.element.querySelector(`.tool[data-name='${this.rightTool}']`).classList.add("selected-right");
  }
  
  renderPalette() {
    const canvas = this.element.querySelector(".palette");
    const context = canvas.getContext("2d");
    context.imageSmoothingEnabled = false;
    if (this.tilesheetImage) {
      const colw = this.tilesheetImage.width >> 4;
      const rowh = this.tilesheetImage.height >> 4;
      const srcx = (this.paletteChoice & 15) * colw;
      const srcy = (this.paletteChoice >> 4) * rowh;
      context.drawImage(
        this.tilesheetImage,
        srcx, srcy, colw, rowh,
        0, 0, canvas.width, canvas.height
      );
    } else {
      context.fillStyle = "#888";
      context.fillRect(0, 0, canvas.width, canvas.height);
    }
  }

  //TODO this is a quick-n-dirty palette. Could do a lot cleaner.  
  onEditPalette() {
    if (!this.tilesheetImage) return;
    const container = this.dom.spawnModal();
    const img = this.dom.spawn(container, "IMG", {
      src: this.tilesheetImage.src,
      style: "width: 384px; image-rendering: crisp-edges",
      "on-click": (event) => this.onPaletteModalClick(event),
    });
  }
  
  onPaletteModalClick(event) {
    const targetBox = event.target.getBoundingClientRect();
    const x = event.clientX - targetBox.left;
    const y = event.clientY - targetBox.top;
    if ((x < 0) || (x >= 384)) return;
    if ((y < 0) || (y >= 384)) return;
    const tileid = Math.floor(y / 24) * 16 + Math.floor(x / 24);
    if ((tileid < 0) || (tileid >= 256)) return;
    this.dom.popModal();
    this.mapService.setPaletteChoice(tileid);
  }
  
  onMapEvent(e) {
    switch (e.event) {
      case "tilesheetImage": {
          if (this.tilesheetImage === e.tilesheetImage) return;
          this.tilesheetImage = e.tilesheetImage;
          this.renderPalette();
        } break;
      case "paletteChoice": {
          if (this.paletteChoice === e.paletteChoice) return;
          this.paletteChoice = e.paletteChoice;
          this.renderPalette();
        } break;
      case "leftTool": {
          if (this.leftTool === e.tool) return;
          this.leftTool = e.tool;
          this.refreshToolHighlight();
        } break;
      case "rightTool": {
          if (this.rightTool === e.tool) return;
          this.rightTool = e.tool;
          this.refreshToolHighlight();
        } break;
      case "sizeChanged": {
          this.element.querySelector("button.resize").innerText = `Size: ${this.map.w},${this.map.h}`;
        } break;
    }
  }
}
