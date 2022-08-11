/* PoiModal.js
 */
 
import { Dom } from "../util/Dom.js";
import { FullmoonMap } from "./FullmoonMap.js";

export class PoiModal {
  static getDependencies() {
    return [HTMLElement, Dom];
  }
  constructor(element, dom) {
    this.element = element;
    this.dom = dom;
    
    // Caller should set directly:
    onsubmit = (poi) => {};
    
    this.poi = {
      x: 0,
      y: 0,
      q: [0, 0, 0, 0],
      qp: "",
    };
    
    this.buildUi();
  }
  
  setupNew(x, y) {
    this.poi.x = x;
    this.poi.y = y;
    this.populateUi("Create");
  }
  
  setupEdit(poi) {
    this.poi.x = poi.x;
    this.poi.y = poi.y;
    this.poi.q = [...poi.q];
    this.poi.qp = poi.qp;
    this.rebuildQFields();
    this.populateUi("Save");
  }
  
  buildUi() {
    this.element.innerHTML = "";
    
    const form = this.dom.spawn(this.element, "FORM", {
      "on-submit": (event) => this.onSubmitForm(event),
      "on-input": (event) => this.onInput(event),
    });
    
    const table = this.dom.spawn(form, "TABLE");
    this.spawnRow(table, "x", "X");
    this.spawnRow(table, "y", "Y");
    this.spawnRow(table, "q0", "Type");
    this.spawnRow(table, "q1", "q[1]");
    this.spawnRow(table, "q2", "q[2]");
    this.spawnRow(table, "q3", "q[3]");
    this.spawnRow(table, "qp", "qp");
    
    this.dom.spawn(form, "INPUT", { type: "submit" });
  }
  
  spawnRow(table, key, label) {
    const tr = this.dom.spawn(table, "TR", { "data-key": key });
    this.dom.spawn(tr, "TD", label, { "data-key": key });
    const td = this.dom.spawn(tr, "TD");
    
    const params = {};
    if (key === "qp") {
      params.type = "text";
    } else if (key === "q0") {
      const select = this.dom.spawn(td, "SELECT", { name: key, "on-change": () => this.onQ0Changed() });
      for (let i=0; i<256; i++) {
        this.dom.spawn(select, "OPTION", FullmoonMap.POI_NAMES[i] || i.toString(), { value: i });
      }
      return;
    } else {
      params.type = "number";
      params.min = 0;
      params.max = 255;
    }
    
    let value = "";
    if (this.poi) switch (key) {
      case "q1": value = this.poi.q[1]; break;
      case "q2": value = this.poi.q[2]; break;
      case "q3": value = this.poi.q[3]; break;
      case "qp": value = this.poi.qp; break;
      case "q12_16": {
          let s16 = (this.poi.q[1] << 8) | this.poi.q[2];
          if (s16 & 0x8000) s16 |= ~0xffff;
          value = s16;
        } break;
    }
    
    this.dom.spawn(td, "INPUT", params, { name: key, value });
  }
  
  populateUi(submitLabel) {
    this.element.querySelector("input[name='x']").value = this.poi.x;
    this.element.querySelector("input[name='y']").value = this.poi.y;
    this.element.querySelector("select[name='q0']").value = this.poi.q[0];
    switch (FullmoonMap.POI_NAMES[this.poi.q[0]]) {
      case "EDGE_DOOR": {
          let s16 = (this.poi.q[1] << 8) | this.poi.q[2];
          if (s16 & 0x8000) s16 |= ~0xffff;
          this.element.querySelector("input[name='q12_16']").value = s16;
        } break;
      default: {
          this.element.querySelector("input[name='q1']").value = this.poi.q[1];
          this.element.querySelector("input[name='q2']").value = this.poi.q[2];
          this.element.querySelector("input[name='q3']").value = this.poi.q[3];
        } break;
    }
    this.element.querySelector("input[name='qp']").value = this.poi.qp;
    this.element.querySelector("input[type='submit']").value = submitLabel;
  }
  
  onInput(event) {
    switch (event.target.name) {
      case "x": this.poi.x = +event.target.value; break;
      case "y": this.poi.y = +event.target.value; break;
      case "q0": this.poi.q[0] = +event.target.value; break;
      case "q1": this.poi.q[1] = +event.target.value; break;
      case "q2": this.poi.q[2] = +event.target.value; break;
      case "q3": this.poi.q[3] = +event.target.value; break;
      case "qp": this.poi.qp = event.target.value; break;
      case "q12_16": {
          const s16 = +event.target.value;
          this.poi.q[1] = (s16 >> 8) & 0xff;
          this.poi.q[2] = s16 & 0xff;
        } break;
    }
  }
  
  onSubmitForm(event) {
    event.preventDefault();
    this.poi.x = +this.element.querySelector("input[name='x']").value;
    this.poi.y = +this.element.querySelector("input[name='y']").value;
    this.poi.q[0] = +this.element.querySelector("select[name='q0']").value;
    switch (FullmoonMap.POI_NAMES[this.poi.q[0]]) {
      case "EDGE_DOOR": {
          const s16 = +this.element.querySelector("input[name='q12_16']").value;
          this.poi.q[1] = (s16 >> 8) & 0xff;
          this.poi.q[2] = s16 & 0xff;
          this.poi.q[3] = 0;
        } break;
      default: {
          this.poi.q[1] = +this.element.querySelector("input[name='q1']").value;
          this.poi.q[2] = +this.element.querySelector("input[name='q2']").value;
          this.poi.q[3] = +this.element.querySelector("input[name='q3']").value;
        }
    }
    this.poi.qp = this.element.querySelector("input[name='qp']").value;
    this.onsubmit(this.poi);
    this.dom.popModal();
  }
  
  rebuildQFields() {
    const table = this.element.querySelector("table");
    table.querySelector("tr[data-key='q1']")?.remove();
    table.querySelector("tr[data-key='q2']")?.remove();
    table.querySelector("tr[data-key='q3']")?.remove();
    table.querySelector("tr[data-key='qp']")?.remove();
    table.querySelector("tr[data-key='q12_16']")?.remove();
    const q0 = this.poi.q[0];
    
    const fourInputs = (q1, q2, q3, qp) => {
      this.spawnRow(table, "q1", q1 || "q[1]");
      this.spawnRow(table, "q2", q2 || "q[2]");
      this.spawnRow(table, "q3", q3 || "q[3]");
      this.spawnRow(table, "qp", qp || "q[4]");
    };
      
    switch (FullmoonMap.POI_NAMES[q0]) {
      case "START": fourInputs("XXX", "XXX", "XXX", "XXX"); break;
      case "DOOR": fourInputs("x", "y", "XXX", "map"); break;
      case "SPRITE": fourInputs("bv[0]", "bv[1]", "bv[2]", "sprite"); break;
      case "TREADLE": fourInputs("q[1]", "q[2]", "q[3]", "function"); break;
      case "VISIBILITY": fourInputs("q[1]", "q[2]", "q[3]", "function"); break;
      case "PROXIMITY": fourInputs("q[1]", "q[2]", "q[3]", "function"); break;
      case "EDGE_DOOR": {
          this.spawnRow(table, "q12_16", "offset");
          this.spawnRow(table, "qp", "map");
        } break;
      case "INTERIOR_DOOR": fourInputs("x", "y", "XXX", "XXX"); break;
      default: fourInputs(); break;
    }
  }
  
  onQ0Changed() {
    this.rebuildQFields();
  }
}
