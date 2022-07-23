/* MapExtras.js
 * Extra content for the header bar, when editing a FullmoonMap.
 */
 
import { Dom } from "../util/Dom.js";
import { FullmoonMap } from "./FullmoonMap.js";
import { MapService } from "./MapService.js";

export class MapExtras {
  static getDependencies() {
    return [HTMLElement, Dom, MapService];
  }
  constructor(element, dom, mapService) {
    this.element = element;
    this.dom = dom;
    this.mapService = mapService;
    
    this.mapSubscription = this.mapService.subscribe(e => this.onMapEvent(e));
    
    this.buildUi();
  }
  
  setup(map) {
    this.map = map;
  }
  
  onRemoveFromDom() {
    this.mapService.unsubscribe(this.mapSubscription);
  }
  
  buildUi() {
    this.element.innerHTML = "";
    this.dom.spawn(this.element, "DIV", ["positionTattle"]);
    //TODO scale control
    //TODO resize
    //TODO enter general metadata view
    //TODO set tilesheet
  }
  
  onHoverPosition(event) {
    this.element.querySelector(".positionTattle").innerText = `${event.x},${event.y}`;
  }
  
  onMapEvent(event) {
    switch (event.event) {
      case "hoverPosition": this.onHoverPosition(event); break;
    }
  }
}
