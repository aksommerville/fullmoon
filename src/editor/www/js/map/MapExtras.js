/* MapExtras.js
 * Extra content for the header bar, when editing a FullmoonMap.
 */
 
import { Dom } from "../util/Dom.js";
import { FullmoonMap } from "./FullmoonMap.js";
import { MapService } from "./MapService.js";

export class MapExtras {
  static getDependencies() {
    return [HTMLElement, Dom, MapService, "discriminator"];
  }
  constructor(element, dom, mapService, discriminator) {
    this.element = element;
    this.dom = dom;
    this.mapService = mapService;
    this.discriminator = discriminator;
    
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
    this.dom.spawn(this.element, "DIV", ["positionTattle"], "000,000");
    this.dom.spawn(this.element, "INPUT", {
      type: "range", min: 1, max: 64, value: this.mapService.tileSize,
      name: "tilesize",
      "on-input": (event) => this.onTileSizeChange(event)
    });
    this.addRenderFeatureToggle("cellLines");
    this.addRenderFeatureToggle("screenLines");
    //TODO enter general metadata view
    //TODO set tilesheet
  }
  
  addRenderFeatureToggle(name) {
    const id = `${this.discriminator}-featureToggle-${name}`;
    const input = this.dom.spawn(this.element, "INPUT", ["featureToggle"], { type: "checkbox", name, id, "on-change": () => this.onFeatureToggled() });
    if (this.mapService.renderFeatures.includes(name)) {
      input.checked = true;
    }
    this.dom.spawn(this.element, "LABEL", ["featureToggle"], name, { for: id });
  }
  
  onHoverPosition(event) {
    this.element.querySelector(".positionTattle").innerText = this.reprPosition(event.x, event.y);
  }
  
  reprPosition(x, y) {
    // We want a fixed width. So it's styled with font-family:monospace, and we'll pad each number to 3 digits.
    return x.toString().padStart(3, '0') + "," + y.toString().padStart(3, '0');
  }
  
  onMapEvent(event) {
    switch (event.event) {
      case "hoverPosition": this.onHoverPosition(event); break;
    }
  }
  
  onTileSizeChange(event) {
    this.mapService.setTileSize(+event.target.value);
  }
  
  onFeatureToggled() {
    const enabled = [];
    for (const checkbox of this.element.querySelectorAll("input.featureToggle")) {
      if (checkbox.checked) enabled.push(checkbox.name);
    }
    this.mapService.setRenderFeatures(enabled);
  }
}
