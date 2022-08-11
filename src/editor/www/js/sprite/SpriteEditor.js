/* SpriteEditor.js
 */
 
import { Dom } from "../util/Dom.js";
import { Sprite } from "./Sprite.js";
import { ResourceService } from "../service/ResourceService.js";

export class SpriteEditor {
  static getDependencies() {
    return [HTMLElement, Dom, ResourceService];
  }
  constructor(element, dom, resourceService) {
    this.element = element;
    this.dom = dom;
    this.resourceService = resourceService;
    
    this.sprite = null;
    
    this.buildUi();
  }
  
  setup(sprite) {
    this.sprite = sprite;
    this.populateUi();
  }
  
  buildUi() {
    this.element.innerHTML = "";
    const table = this.dom.spawn(this.element, "TABLE", { "on-input": (event) => this.onInput(event) });
    for (const key of Sprite.FIELD_NAMES) {
      this.addRow(table, key, Sprite.FIELD_COMMENTS[key]);
    }
    // The Sprite model retains unknown fields, but I'm not doing anything with them here.
  }
  
  addRow(table, key, comment) {
    const tr = this.dom.spawn(table, "TR");
    this.dom.spawn(tr, "TD", key);
    const tdv = this.dom.spawn(tr, "TD");
    this.dom.spawn(tdv, "INPUT", { name: key });
    this.dom.spawn(tr, "TD", comment || "");
  }
  
  populateUi() {
    for (const key of Sprite.FIELD_NAMES) {
      const input = this.element.querySelector(`input[name='${key}']`);
      input.value = this.sprite?.[key] || "";
    }
  }
  
  onInput(event) {
    if (!this.sprite) return;
    if (!Sprite.FIELD_NAMES.includes(event?.target?.name)) return;
    this.sprite[event.target.name] = event.target.value || "";
    this.resourceService.dirty(this.sprite.path, () => this.sprite.encode()).then(() => {
    }).catch((e) => {
      if (e !== "replaced") {
        console.error(e);
      }
    });
  }
}
