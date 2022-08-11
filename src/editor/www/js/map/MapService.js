/* MapService.js
 * Manages the model-level editing of FullmoonMap objects.
 * This is the code that knows what a pencil is, etc.
 * Kind of painted myself into an architectural corner that I'm not smart enough to paint my way out of.
 * This should be instantiated once per instance of MapEditor, and shared across the various Map UI controllers.
 * But to make it reachable to all of them, I'm making it a singleton, and its state gets wiped each time a new MapEditor instantiates.
 *
 * Events sent to subscribers:
 *  { event:"tilesheet", tilesheet:Tilesheet }
 *  { event:"tilesheetImage", tilesheetImage:Image }
 *  { event:"paletteChoice", paletteChoice:0..255 }
 *  { event:"leftTool", tool:string } // effective tool, ie accounts for modifier keys
 *  { event:"rightTool", tool:string } // ''
 *  { event:"dirty" } // changes in progress (recommend redraw but not save)
 *  { event:"finishEdit" } // stroke complete and something changed (recommend save)
 *  { event:"promptForNewPoi", x, y } // request modal for creating a POI. MapEditor should handle it.
 *  { event:"promptForEditPoi", index, poi } // same idea
 *  { event:"tilesheetChanged" } // signal for MapEditor to reload the sheet
 *  { event:"sizeChanged" }
 *  { event:"hoverPosition", x, y } // cell the cursor hovers on
 *  { event:"region", region:string }
 *  { event:"tileSize", tileSize: >=1 }
 *  { event:"renderFeatures", features:array } // "cellLines","screenLines"
 *  { event:"focusCell", x, y } // activated interior door
 */
 
import { FullmoonMap } from "./FullmoonMap.js";
import { ResourceService } from "../service/ResourceService.js";
import { Dom } from "../util/Dom.js";
 
export class MapService {
  static getDependencies() {
    return [Window, ResourceService, Dom];
  }
  constructor(window, resourceService, dom) {
    this.window = window;
    this.resourceService = resourceService;
    this.dom = dom;
    
    this.nextSubscriberId = 1;
    this.reset(null);
    
    this.window.addEventListener("keydown", (event) => this.onKeyDown(event));
    this.window.addEventListener("keyup", (event) => this.onKeyUp(event));
  }
  
  reset(map) {
    this.map = map;
    this.tilesheet = null;
    this.tilesheetImage = null; // img
    this.paletteChoice = 0; // 0..255
    if (!this.leftTool) this.leftTool = "pencil";
    if (!this.rightTool) this.rightTool = "poiedit";
    this.leftToolAnchor = this.leftTool;
    this.rightToolAnchor = this.rightTool;
    this.toolInProgress = null; // while mouse button down
    this.strokeDirty = false;
    this.poiDragIndex = -1;
    this.hoverX = -1;
    this.hoverY = -1;
    if (!this.tileSize) this.tileSize = 32;
    if (!this.renderFeatures) this.renderFeatures = ["cellLines", "screenLines"];
    this.subscribers = [];
  }
  
  subscribe(cb) {
    const id = this.nextSubscriberId++;
    this.subscribers.push({ id, cb });
    return id;
  }
  
  unsubscribe(id) {
    const p = this.subscribers.findIndex(s => s.id === id);
    if (p < 0) return;
    this.subscribers.splice(p, 1);
  }
  
  broadcast(event) {
    for (const { cb } of this.subscribers) {
      cb(event);
    }
  }
  
  /* Events that trigger state broadcasts.
   *********************************************************/
   
  setTilesheet(tilesheet) {
    if (tilesheet === this.tilesheet) return;
    this.tilesheet = tilesheet;
    this.broadcast({
      event: "tilesheet",
      tilesheet,
    });
  }
  
  tilesheetImageReady(tilesheetImage) {
    if (tilesheetImage === this.tilesheetImage) return;
    this.tilesheetImage = tilesheetImage;
    this.broadcast({
      event: "tilesheetImage",
      tilesheetImage,
    });
  }
  
  setPaletteChoice(n) {
    if ((n < 0) || (n > 255)) return;
    if (n === this.paletteChoice) return;
    this.paletteChoice = n;
    this.broadcast({
      event: "paletteChoice",
      paletteChoice: n,
    });
  }
  
  setLeftTool(tool, transient) {
    if (!MapService.TOOLS.includes(tool)) return;
    if (tool === this.leftTool) return;
    this.leftTool = tool;
    if (!transient) this.leftToolAnchor = tool;
    this.broadcast({
      event: "leftTool",
      tool,
    });
  }
  
  setRightTool(tool, transient) {
    if (!MapService.TOOLS.includes(tool)) return;
    if (tool === this.rightTool) return;
    this.rightTool = tool;
    if (!transient) this.rightToolAnchor = tool;
    this.broadcast({
      event: "rightTool",
      tool,
    });
  }
  
  setMapCell(x, y, tileid) {
    if (!this.map || (x < 0) || (y < 0) || (x >= this.map.w) || (y >= this.map.h) || (tileid < 0) || (tileid > 255)) return;
    const p = y * this.map.w + x;
    if (this.map.cells[p] === tileid) return;
    if (this.toolInProgress) this.strokeDirty = true;
    this.map.cells[p] = tileid;
    this.broadcast({ event: "dirty" });
  }
  
  finishEdit() {
    if (!this.strokeDirty) return;
    this.strokeDirty = false;
    this.broadcast({ event: "finishEdit" });
  }
  
  setTilesheetName(name) {
    if (!this.map) return;
    if (this.map.tilesheet === name) return;
    this.map.tilesheet = name;
    this.broadcast({ event: "tilesheetChanged" });
    this.broadcast({ event: "dirty" });
    this.broadcast({ event: "finishEdit" });
  }
  
  setHoverPosition(x, y) {
    if ((x === this.hoverX) && (y === this.hoverY)) return;
    this.hoverX = x;
    this.hoverY = y;
    this.broadcast({ event: "hoverPosition", x, y });
  }
  
  setRegion(name) {
    if (!this.map || (this.map.region === name)) return;
    this.map.region = name;
    this.broadcast({ event: "region", region: name });
    this.broadcast({ event: "dirty" });
    this.broadcast({ event: "finishEdit" });
  }
  
  setTileSize(tileSize) {
    if (typeof(tileSize) !== "number") return;
    if (!tileSize || tileSize<1) return;
    this.tileSize = tileSize;
    this.broadcast({ event: "tileSize", tileSize });
  }
  
  setRenderFeatures(features) {
    features = [...features].sort();
    let same = true;
    if (features.length === this.renderFeatures.length) {
      for (let i=features.length; i-->0;) if (features[i] !== this.renderFeatures[i]) {
        same = false;
        break;
      }
    } else same = false;
    if (same) return;
    this.renderFeatures = features;
    this.broadcast({ event: "renderFeatures", features });
  }
  
  /* Resize.
   ***********************************************************/
   
  resize(w, h) {
    if (!this.map) return;
    if ((typeof(w) !== "number") || (typeof(h) !== "number")) return;
    if ((w < 1) || (w > 255) || (w % 9)) return;
    if ((h < 1) || (h > 255) || (h % 5)) return;
    if ((w === this.map.w) && (h === this.map.h)) return;
    this.map.resize(w, h);
    this.broadcast({ event:"sizeChanged" });
  }
  
  /* Set map cell with randomization on the focus point, and neighbor joining in the surrounding 3x3 area.
   * ie the Rainbow Pencil.
   *******************************************************************/
   
  _joinNeighbors(x, y, randomize) {
    if (x < 0) return;
    if (y < 0) return;
    if (x >= this.map.w) return;
    if (y >= this.map.h) return;
    const p = y * this.map.w + x;
    const tileid0 = this.map.cells[p];
    const tile0 = this.tilesheet.tiles[tileid0];
    
    // No group? It can only be whatever it is.
    if (!tile0.group) return;
    
    // Compose the neighbor mask.
    let neighbors = 0;
    for (let bit=0x80,dy=-1; dy<=1; dy++) {
      const ny = y + dy; // don't skip if OOB; we need to step (bit)
      for (let dx=-1; dx<=1; dx++) {
        if (!dx && !dy) continue;
        const nx = x + dx;
        if ((nx >= 0) && (ny >= 0) && (nx < this.map.w) && (ny < this.map.h)) {
          if (this.tilesheet.tiles[this.map.cells[ny * this.map.w + nx]].group === tile0.group) {
            neighbors |= bit;
          }
        }
        bit >>= 1;
      }
    }
    
    // Find the best fit tile.
    this.map.cells[p] = this.tilesheet.findBestFit(tileid0, neighbors, randomize);
  }
   
  setMapCellWithInference(x, y, tileid) {
    if (!this.tilesheet) return this.setMapCell(x, y, tileid);
    if (!this.map || (x < 0) || (y < 0) || (x >= this.map.w) || (y >= this.map.h) || (tileid < 0) || (tileid > 255)) return;
    const p = y * this.map.w + x;
    const baseTile = this.tilesheet.tiles[tileid];
    
    // Provisionally set this tile to the exact stated value, so the neighbors can see.
    this.map.cells[p] = tileid;
    
    // Reconsider each neighbor. (including the focus cell)
    // Randomize the focus but not the neighbors, if they already have a correct tile.
    // Note that we do this even if the incoming tile has no group -- the neighbors might.
    for (let dx=-1; dx<=1; dx++) {
      for (let dy=-1; dy<=1; dy++) {
        this._joinNeighbors(x + dx, y + dy, !dx && !dy);
      }
    }
    
    // It's hard to assert that anything actually changed, but assume that something did, whatever.
    if (this.toolInProgress) this.strokeDirty = true;
    this.broadcast({ event: "dirty" });
  }
  
  /* The finger tool: Context sensitive "activation", eg follow doors.
   ********************************************************************/
   
  navigate(name, x, y) {
    const path = `/res/map/${name}.txt`;
    if ((typeof(x) === "number") && (typeof(y) === "number")) {
      this.resourceService.editResource(path, name, { x, y });
    } else {
      this.resourceService.editResource(path, name);
    }
  }
   
  activatePoi(poi) {
    switch (FullmoonMap.POI_NAMES[poi.q[0]]) {
      case "DOOR": this.navigate(poi.qp, poi.q[1], poi.q[2]); return true;
      case "EDGE_DOOR": this.navigate(poi.qp); return true;
      case "INTERIOR_DOOR": this.broadcast({ event: "focusCell", x: poi.q[1], y: poi.q[2] }); return true;
    }
    return false;
  }
   
  activateCell(x, y) {
    if (!this.map) return;
    if ((x < 0) || (y < 0) || (x >= this.map.w) || (y >= this.map.h)) return;
    for (const poi of this.map.pois) {
      if (poi.x !== x) continue;
      if (poi.y !== y) continue;
      if (this.activatePoi(poi)) return;
    }
  }
  
  /* Key events from window so we can track modifier keys.
   * Also using this for tool-switch hotkeys.
   ******************************************************************/
   
  applyTemporaryToolAlias() {
    const variantIndex = (this.controlKeyDown ? 2 : 0) + (this.shiftKeyDown ? 1 : 0);
    const leftVariant = MapService.TOOL_VARIANTS[this.leftToolAnchor]?.[variantIndex] || this.leftTool;
    const rightVariant = MapService.TOOL_VARIANTS[this.rightToolAnchor]?.[variantIndex] || this.rightTool;
    if (leftVariant !== this.leftTool) {
      this.setLeftTool(leftVariant, true);
    }
    if (rightVariant !== this.rightTool) {
      this.setRightTool(rightVariant, true);
    }
  }
  
  hasKeyboardFocus() {
    if (!this.dom.document.querySelector(".MapEditor")) return false;
    if (this.dom.hasModal()) return false;
    return true;
  }
   
  onKeyDown(event) {
    switch (event.key) {
      case "Control": {
          if (this.controlKeyDown) return;
          this.controlKeyDown = true;
          this.applyTemporaryToolAlias();
        } break;
      case "Shift": {
          if (this.shiftKeyDown) return;
          this.shiftKeyDown = true;
          this.applyTemporaryToolAlias();
        } break;
    }
    if (this.hasKeyboardFocus()) {
      switch (event.key) {
        case "p": this.setLeftTool("pencil"); event.stopPropagation(); return; // P for Pencil
        case "o": this.setLeftTool("pickup"); event.stopPropagation(); return; // O like GIMP
        case "r": this.setLeftTool("rainbow"); event.stopPropagation(); return; // R for Rainbow
        case "f": this.setLeftTool("finger"); event.stopPropagation(); return; // F for Finger
        case "n": this.setLeftTool("poinew"); event.stopPropagation(); return; // N for New POI
        case "m": this.setLeftTool("poimove"); event.stopPropagation(); return; // M for Move POI
        case "d": this.setLeftTool("poidelete"); event.stopPropagation(); return; // D for Delete POI
        case "e": this.setLeftTool("poiedit"); event.stopPropagation(); return; // E for Edit POI
      }
    }
  }
  
  onKeyUp(event) {
    switch (event.key) {
      case "Control": {
          if (!this.controlKeyDown) return;
          this.controlKeyDown = false;
          this.applyTemporaryToolAlias();
        } break;
      case "Shift": {
          if (!this.shiftKeyDown) return;
          this.shiftKeyDown = false;
          this.applyTemporaryToolAlias();
        } break;
    }
  }
  
  /* Tool hooks.
   **************************************************************/
   
  pickupMouseMove(x, y) {
    this.setPaletteChoice(this.map.cells[y * this.map.w + x]);
  }
  
  pickupMouseDown(x, y) {
    this.pickupMouseMove(x, y);
  }
  
  pencilMouseMove(x, y) {
    this.setMapCell(x, y, this.paletteChoice);
  }
   
  pencilMouseDown(x, y) {
    this.pencilMouseMove(x, y);
  }
   
  rainbowMouseMove(x, y) {
    this.setMapCellWithInference(x, y, this.paletteChoice);
  }
  
  rainbowMouseDown(x, y) {
    this.rainbowMouseMove(x, y);
  }
  
  pickupMouseUp() {}
  pencilMouseUp() {}
  rainbowMouseUp() {}
  poimoveMouseUp() {}
  
  fingerMouseDown(x, y) {
    this.activateCell(x, y);
  }
  
  poinewMouseDown(x, y) {
    this.broadcast({
      event: "promptForNewPoi",
      x, y,
    });
  }
  
  poieditMouseDown(x, y) {
    const index = this.map.pois.findIndex((poi) => ((poi.x === x) && (poi.y === y)));
    if (index < 0) return;
    this.broadcast({
      event: "promptForEditPoi",
      index,
      poi: this.map.pois[index],
    });
  }
  
  poideleteMouseDown(x, y) {
    const index = this.map.pois.findIndex((poi) => ((poi.x === x) && (poi.y === y)));
    if (index < 0) return;
    this.map.pois.splice(index, 1);
    this.strokeDirty = true;
    this.broadcast({ event: "dirty" });
  }
  
  poimoveMouseMove(x, y) {
    if (this.poiDragIndex < 0) return;
    const poi = this.map.pois[this.poiDragIndex];
    poi.x = x;
    poi.y = y;
    this.strokeDirty = true;
    this.broadcast({ event: "dirty" });
  }
  
  poimoveMouseDown(x, y) {
    this.poiDragIndex = this.map.pois.findIndex(poi => ((poi.x === x) && (poi.y === y)));
    if (this.poiDragIndex < 0) return;
  }
  
  /* Mouse events from MapEditor, dispatch.
   * Caller should avoid sending redundant motion, and must provide in-bounds coords.
   * ...on second thought, we will check bounds, why not.
   * But if you want clamping, which you probly do, that's not my problem.
   ***************************************************************/
   
  mouseDown(button, x, y) {
    if ((x < 0) || (y < 0) || (x >= this.map.w) || (y >= this.map.h)) return;
    if (this.toolInProgress) return;
    this.strokeDirty = false;
    let tool;
    switch (button) {
      case 0: tool = this.leftTool; break;
      case 1: // fucking stupid motherfucker firefox treats Shift-Right-Click as "non-overridable contextmenu". So let's allow middle button too.
      case 2: tool = this.rightTool; break;
    }
    if (!tool) return;
    this.toolInProgress = tool;
    switch (this.toolInProgress) {
      case "pickup": this.pickupMouseDown(x, y); break;
      case "pencil": this.pencilMouseDown(x, y); break;
      case "rainbow": this.rainbowMouseDown(x, y); break;
      case "finger": this.fingerMouseDown(x, y); this.toolInProgress = null; break;
      case "poinew": this.poinewMouseDown(x, y); this.toolInProgress = null; break;
      case "poiedit": this.poieditMouseDown(x, y); this.toolInProgress = null; break;
      case "poidelete": this.poideleteMouseDown(x, y); this.toolInProgress = null; break;
      case "poimove": this.poimoveMouseDown(x, y); break;
    }
  }
  
  mouseUp() {
    const tool = this.toolInProgress;
    this.toolInProgress = null;
    switch (tool) {
      case "pickup": this.pickupMouseUp(); break;
      case "pencil": this.pencilMouseUp(); break;
      case "rainbow": this.rainbowMouseUp(); break;
      case "poimove": this.poimoveMouseUp(); break;
    }
    this.finishEdit();
  }
  
  mouseMove(x, y) {
    this.setHoverPosition(x, y);
    if ((x < 0) || (y < 0) || (x >= this.map.w) || (y >= this.map.h)) return;
    switch (this.toolInProgress) {
      case "pickup": this.pickupMouseMove(x, y); break;
      case "pencil": this.pencilMouseMove(x, y); break;
      case "rainbow": this.rainbowMouseMove(x, y); break;
      case "poimove": this.poimoveMouseMove(x, y); break;
    }
  }
}

MapService.singleton = true;

MapService.TOOLS = [
  "pickup", "pencil", "rainbow", "finger",
  "poinew", "poiedit", "poidelete", "poimove",
];

MapService.TOOL_VARIANTS = {
          // natural, shift, control, shift+control
  pickup: ["pickup", "pickup", "pencil", "pencil"],
  pencil: ["pencil", "rainbow", "pickup", "pickup"],
  rainbow: ["rainbow", "pencil", "pickup", "pickup"],
  finger: ["finger", "finger", "pickup", "pickup"],
  poinew: ["poinew", "poinew", "poinew", "poinew"],
  poiedit: ["poiedit", "poimove", "poinew", "poidelete"],
  poidelete: ["poidelete", "poidelete", "poidelete", "poidelete"],
  poimove: ["poimove", "poimove", "poimove", "poimove"],
};
