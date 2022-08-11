/* DataFactory.js
 * Generic interface to the various model objects we can work with, and their UI controllers.
 */
 
import { Decoder } from "../util/Decoder.js";
import { FullmoonMap } from "../map/FullmoonMap.js";
import { MapEditor } from "../map/MapEditor.js";
import { MapToolbox } from "../map/MapToolbox.js";
import { MapExtras } from "../map/MapExtras.js";
import { Tilesheet } from "../tilesheet/Tilesheet.js";
import { TilesheetEditor } from "../tilesheet/TilesheetEditor.js";
import { TilesheetToolbox } from "../tilesheet/TilesheetToolbox.js";
import { TilesheetExtras } from "../tilesheet/TilesheetExtras.js";
import { Sprite } from "../sprite/Sprite.js";
import { SpriteEditor } from "../sprite/SpriteEditor.js";
 
export class DataFactory {
  static getDependencies() {
    return [];
  }
  constructor() {
  }
  
  /* Public API.
   ***************************************************************/
  
  decode(serial, path) {
    if (path.startsWith("/res/map/")) return new FullmoonMap(serial, path);
    if (path.match(/^\/res\/image\/.*_props.txt$/)) return new Tilesheet(serial, path);
    if (path.startsWith("/res/sprite/")) return new Sprite(serial, path);
    console.warn(`DataFactory.decode, unimplemented data type`, { serial, path });
    return null;
  }
  
  controllerClassForData(data) {
    if (data instanceof FullmoonMap) return MapEditor;
    if (data instanceof Tilesheet) return TilesheetEditor;
    if (data instanceof Sprite) return SpriteEditor;
    return null;
  }
  
  toolboxClassForData(data) {
    if (data instanceof FullmoonMap) return MapToolbox;
    if (data instanceof Tilesheet) return TilesheetToolbox;
    return null;
  }
  
  extrasClassForData(data) {
    if (data instanceof FullmoonMap) return MapExtras;
    if (data instanceof Tilesheet) return TilesheetExtras;
    return null;
  }
}

DataFactory.singleton = true;
