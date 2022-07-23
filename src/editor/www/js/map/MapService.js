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
 */
 
import { FullmoonMap } from "./FullmoonMap.js";
 
export class MapService {
  static getDependencies() {
    return [Window];
  }
  constructor(window) {
    this.window = window;
    
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
    this.leftTool = "pencil";
    this.rightTool = "poiedit";
    this.leftToolAnchor = this.leftTool;
    this.rightToolAnchor = this.rightTool;
    this.toolInProgress = null; // while mouse button down
    this.strokeDirty = false;
    this.poiDragIndex = -1;
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
  
  /* Key events from window so we can track modifier keys.
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
  "pickup", "pencil", "rainbow", 
  "poinew", "poiedit", "poidelete", "poimove",
];

MapService.TOOL_VARIANTS = {
          // natural, shift, control, shift+control
  pickup: ["pickup", "pickup", "pencil", "pencil"],
  pencil: ["pencil", "rainbow", "pickup", "pickup"],
  rainbow: ["rainbow", "pencil", "pickup", "pickup"],
  poinew: ["poinew", "poinew", "poinew", "poinew"],
  poiedit: ["poiedit", "poimove", "poinew", "poidelete"],
  poidelete: ["poidelete", "poidelete", "poidelete", "poidelete"],
  poimove: ["poimove", "poimove", "poimove", "poimove"],
};
