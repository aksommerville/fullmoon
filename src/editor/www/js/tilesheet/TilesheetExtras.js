/* TilesheetExtras.js
 */
 
import { Dom } from "../util/Dom.js";
import { Tilesheet } from "./Tilesheet.js";

export class TilesheetExtras {
  static getDependencies() {
    return [HTMLElement, Dom];
  }
  constructor(element, dom) {
    this.element = element;
    this.dom = dom;
    
    this.tilesheet = null;
    
    this.buildUi();
  }
  
  setup(tilesheet) {
    this.tilesheet = tilesheet;
  }
  
  buildUi() {
    this.element.innerHTML = "";
  }
}
