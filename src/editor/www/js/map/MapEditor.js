/* MapEditor.js
 */
 
import { Dom } from "../util/Dom.js";
import { FullmoonMap } from "./FullmoonMap.js";
import { ResourceService } from "../service/ResourceService.js";
import { MapService } from "./MapService.js";
import { PoiModal } from "./PoiModal.js";

export class MapEditor {
  static getDependencies() {
    return [HTMLElement, Dom, Window, ResourceService, MapService];
  }
  constructor(element, dom, window, resourceService, mapService) {
    this.element = element;
    this.dom = dom;
    this.window = window;
    this.resourceService = resourceService;
    this.mapService = mapService;
    
    this.map = null;
    this.pixelsPerTile = 32; // output tile size (ie zoom)
    this.srcTileWidth = 8; // tile size in sheet. should always be 8, but we'll read from the image
    this.srcTileheight = 8; // ''
    this.tilesheet = null;
    this.tilesheetImage = null;
    this.mapSubscription = null;
    this.mouseListener = null;
    this.mouseX = 0; // in cells, most recent observed position
    this.mouseY = 0;
    
    this.poiIcons = new Image();
    this.poiIcons.src = "/img/poiicons.png";
    
    this.buildUi();
    
    this.resizeObserver = new this.window.ResizeObserver((event) => this.onResize(event));
    this.resizeObserver.observe(this.element);
  }
  
  onRemoveFromDom() {
    delete this.resizeObserver;
    this.mapService.unsubscribe(this.mapSubscription);
    if (this.mouseListener) {
      this.window.removeEventListener("mousemove", this.mouseListener);
      this.mouseListener = null;
    }
  }
  
  buildUi() {
    this.element.innerHTML = "";
    const canvas = this.dom.spawn(this.element, "CANVAS");
    const scroller = this.dom.spawn(this.element, "DIV", ["scroller"], { "on-scroll": (event) => this.onScroll(event.target) });
    const sizer = this.dom.spawn(scroller, "DIV", ["sizer"], {
      "on-mousedown": (event) => this.onMouseDown(event),
      "on-mouseup": (event) => this.onMouseUp(event),
      "on-contextmenu": (event) => event.preventDefault(),
    });
  }
  
  setup(map) {
    this.mapService.reset(map);
    this.mapSubscription = this.mapService.subscribe(e => this.onMapEvent(e));
    this.map = map;
    const fullWidthPixels = this.map.w * this.pixelsPerTile;
    const fullHeightPixels = this.map.h * this.pixelsPerTile;
    const sizer = this.element.querySelector(".sizer");
    sizer.style.width = `${fullWidthPixels}px`;
    sizer.style.height = `${fullHeightPixels}px`;
    this.onTilesheetChanged();
    this.render();
  }
  
  onResize(events) {
    if (events.length < 1) return;
    const event = events[0];
    const canvas = this.element.querySelector("canvas");
    canvas.width = event.contentRect.width;
    canvas.height = event.contentRect.height;
    const scroller = this.element.querySelector(".scroller");
    this.render(scroller.scrollLeft, scroller.scrollTop);
  }
  
  onScroll(scroller) {
    this.render();
  }
  
  render() {
    const canvas = this.element.querySelector("canvas");
    const ctx = canvas.getContext("2d");
    ctx.imageSmoothingEnabled = false;
    ctx.fillStyle = "#444"; // Fill margin with something not black or white, we have this luxury of impossible colors :)
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    if (!this.map) return;
    
    // Render tiles
    const [left, top] = this.pointCanvasFromMap(0, 0);
    const [right, bottom] = this.pointCanvasFromMap(this.map.w, this.map.h);
    let cola = Math.floor(-left / this.pixelsPerTile);
    let colz = Math.floor((canvas.width - left) / this.pixelsPerTile);
    let rowa = Math.floor(-top / this.pixelsPerTile);
    let rowz = Math.floor((canvas.height - top) / this.pixelsPerTile);
    if (cola < 0) cola = 0;
    if (colz >= this.map.w) colz = this.map.w - 1;
    if (rowa < 0) rowa = 0;
    if (rowz >= this.map.h) rowz = this.map.h - 1;
    let rowp = rowa * this.map.w + cola;
    let dsty = rowa * this.pixelsPerTile + top;
    for (let row = rowa; row <= rowz; row++, rowp += this.map.w, dsty += this.pixelsPerTile) {
      let dstx = cola * this.pixelsPerTile + left;
      for (let colp = rowp, col = cola; col <= colz; col++, colp++, dstx += this.pixelsPerTile) {
        if (this.tilesheetImage) {
          const srcx = (this.map.cells[colp] & 15) * this.srcTileWidth;
          const srcy = (this.map.cells[colp] >> 4) * this.srcTileHeight;
          ctx.drawImage(this.tilesheetImage, srcx, srcy, this.srcTileWidth, this.srcTileHeight, dstx, dsty, this.pixelsPerTile, this.pixelsPerTile);
        } else { // shades of red if we have no image
          ctx.fillStyle = `rgb(${this.map.cells[colp]},0,0)`;
          ctx.fillRect(dstx, dsty, this.pixelsPerTile, this.pixelsPerTile);
        }
      }
    }
    
    // Render POI
    for (const poi of this.map.pois) {
      let dstx = left + poi.x * this.pixelsPerTile;
      let dsty = top + poi.y * this.pixelsPerTile;
      const srcx = (poi.q[0] & 15) * 16;
      const srcy = (poi.q[0] >> 4) * 16;
      ctx.drawImage(this.poiIcons, srcx, srcy, 16, 16, dstx, dsty, 16, 16);
    }
    
    // Cell boundaries
    ctx.beginPath();
    let dstx = cola * this.pixelsPerTile + left;
    for (let col = cola; col <= colz; col++, dstx += this.pixelsPerTile) {
      ctx.moveTo(dstx, top);
      ctx.lineTo(dstx, bottom);
    }
    dsty = rowa * this.pixelsPerTile + top;
    for (let row = rowa; row <= rowz; row++, dsty += this.pixelsPerTile) {
      ctx.moveTo(left, dsty);
      ctx.lineTo(right, dsty);
    }
    ctx.strokeStyle = "#0ff";
    ctx.stroke();
    
    // Screen boundaries. There's few enough of these, don't bother finding the view box.
    ctx.beginPath();
    dstx = left;
    for (; dstx < right; dstx += 9 * this.pixelsPerTile) {
      ctx.moveTo(dstx, top);
      ctx.lineTo(dstx, bottom);
    }
    dsty = top;
    for (; dsty < bottom; dsty += 5 * this.pixelsPerTile) {
      ctx.moveTo(left, dsty);
      ctx.lineTo(right, dsty);
    }
    ctx.strokeStyle = "#ff0";
    ctx.stroke();
  }
  
  onMouseDown(event) {
    if (!this.map) return;
    const [col, row] = this.pointMapFromSizer(event.layerX, event.layerY).map(n => Math.floor(n));
    if ((col < 0) || (row < 0) || (col >= this.map.w) || (row >= this.map.h)) return;
    this.mouseX = col;
    this.mouseY = row;
    if (this.mouseListener) { // huh?
      this.window.removeEventListener("mousemove", this.mouseListener);
      this.mouseListener = null;
    }
    this.mouseListener = (event) => this.onMouseMove(event);
    this.window.addEventListener("mousemove", this.mouseListener);
    this.mapService.mouseDown(event.button, col, row);
  }
  
  onMouseMove(event) {
    if (!this.map) return;
    let [col, row] = this.pointMapFromSizer(event.layerX, event.layerY).map(n => Math.floor(n));
    if (col < 0) col = 0;
    else if (col >= this.map.w) col = this.map.w - 1;
    if (row < 0) row = 0;
    else if (row >= this.map.h) row = this.map.h - 1;
    if ((col === this.mouseX) && (row === this.mouseY)) return;
    this.mouseX = col;
    this.mouseY = row;
    this.mapService.mouseMove(col, row);
  }
  
  onMouseUp(event) {
    if (this.mouseListener) {
      this.window.removeEventListener("mousemove", this.mouseListener);
      this.mouseListener = null;
    }
    this.mapService.mouseUp();
  }
  
  onDirty() {
    this.render();
  }
  
  onFinishEdit() {
    this.resourceService.dirty(this.map.path, () => {
      return this.map.encode();
    }).catch(e => {
      if (e === "replaced") return; // yeah yeah yeah
      console.error(`Failed to save map.`, e);
    });
  }
  
  onNewPoi(event) {
    const wrapper = this.dom.spawnModal();
    const controller = this.dom.spawnController(wrapper, PoiModal);
    controller.setupNew(event.x, event.y);
    controller.onsubmit = (poi) => {
      this.map.pois.push(poi);
      this.mapService.broadcast({ event: "dirty" });
      this.mapService.broadcast({ event: "finishEdit" });
    };
  }
  
  onEditPoi(event) {
    const wrapper = this.dom.spawnModal();
    const controller = this.dom.spawnController(wrapper, PoiModal);
    controller.setupEdit(event.poi);
    controller.onsubmit = (poi) => {
      this.map.pois[event.index] = poi;
      this.mapService.broadcast({ event: "dirty" });
      this.mapService.broadcast({ event: "finishEdit" });
    };
  }
  
  onTilesheetChanged() {
    this.tilesheet = null;
    this.resourceService.getResource(`/res/image/${this.map.tilesheet}_props.txt`, true).then((tilesheet) => {
      this.tilesheet = tilesheet;
      this.mapService.setTilesheet(this.tilesheet);
    }).catch((error) => {
      console.error(`Failed to load tilesheet '${this.map.tilesheet}'`, error);
    });
    this.tilesheetImage = null;
    this.resourceService.getImage(`/res/image/${this.map.tilesheet}.png`).then((image) => {
      this.tilesheetImage = image;
      this.srcTileWidth = this.tilesheetImage.naturalWidth >> 4;
      this.srcTileHeight = this.tilesheetImage.naturalHeight >> 4;
      this.render();
      this.mapService.tilesheetImageReady(this.tilesheetImage);
    }).catch((error) => {
      console.error(`Failed to load tilesheet image '${this.map.tilesheet}'`, error);
    });
  }
  
  onSizeChanged() {
    const fullWidthPixels = this.map.w * this.pixelsPerTile;
    const fullHeightPixels = this.map.h * this.pixelsPerTile;
    const sizer = this.element.querySelector(".sizer");
    sizer.style.width = `${fullWidthPixels}px`;
    sizer.style.height = `${fullHeightPixels}px`;
    this.render();
    this.onFinishEdit();
  }
  
  onMapEvent(event) {
    switch (event.event) {
      case "dirty": this.onDirty(); break;
      case "finishEdit": this.onFinishEdit(); break;
      case "promptForNewPoi": this.onNewPoi(event); break;
      case "promptForEditPoi": this.onEditPoi(event); break;
      case "tilesheetChanged": this.onTilesheetChanged(); break;
      case "sizeChanged": this.onSizeChanged(); break;
    }
  }
  
  /* Coordinate transformation.
   * "map" points are tiles.
   * "sizer" points are pixels in the scrolled space.
   * "canvas" points are pixels in the canvas space. These depend on scroll.
   * We'll return map points floating-point in case you need sub-tile precision.
   *************************************************************/
  
  pointCanvasFromMap(mapx, mapy) {
    if (!this.map) return [0, 0];
    const scroller = this.element.querySelector(".scroller");
    const fullw = this.map.w * this.pixelsPerTile;
    const fullh = this.map.h * this.pixelsPerTile;
    const scrollerBounds = scroller.getBoundingClientRect();
    let cx, cy;
    if (scroller.scrollLeftMax <= 0) cx = scrollerBounds.width / 2 - fullw / 2 + mapx * this.pixelsPerTile;
    else cx = mapx * this.pixelsPerTile - scroller.scrollLeft;
    if (scroller.scrollTopMax <= 0) cy = scrollerBounds.height / 2 - fullh / 2 + mapy * this.pixelsPerTile;
    else cy = mapy * this.pixelsPerTile - scroller.scrollTop;
    return [cx, cy];
  }
  
  pointMapFromSizer(sx, sy) {
    if (!this.map) return [0, 0];
    let mx, my;
    const scroller = this.element.querySelector(".scroller");
    const fullw = this.map.w * this.pixelsPerTile;
    const fullh = this.map.h * this.pixelsPerTile;
    const scrollerBounds = scroller.getBoundingClientRect();
    if (scroller.scrollLeftMax <= 0) mx = (sx - scrollerBounds.width / 2 + fullw / 2) / this.pixelsPerTile;
    else mx = sx / this.pixelsPerTile;
    if (scroller.scrollTopMax <= 0) my = (sy - scrollerBounds.height / 2 + fullh / 2) / this.pixelsPerTile;
    else my = sy / this.pixelsPerTile;
    return [mx, my];
  }
}
