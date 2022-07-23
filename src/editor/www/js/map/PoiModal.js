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
    this.populateUi("Save");
  }
  
  buildUi() {
    this.element.innerHTML = "";
    
    const form = this.dom.spawn(this.element, "FORM", { "on-submit": (event) => this.onSubmitForm(event) });
    
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
    const tr = this.dom.spawn(table, "TR");
    this.dom.spawn(tr, "TD", label, { "data-key": key });
    const td = this.dom.spawn(tr, "TD");
    
    const params = {};
    if (key === "qp") {
      params.type = "text";
    } else if (key === "q0") {
      const select = this.dom.spawn(td, "SELECT", { name: key });
      for (let i=0; i<256; i++) {
        this.dom.spawn(select, "OPTION", FullmoonMap.POI_NAMES[i] || i.toString(), { value: i });
      }
      return;
    } else {
      params.type = "number";
      params.min = 0;
      params.max = 255;
    }
    
    this.dom.spawn(td, "INPUT", params, { name: key });
  }
  
  populateUi(submitLabel) {
    this.element.querySelector("input[name='x']").value = this.poi.x;
    this.element.querySelector("input[name='y']").value = this.poi.y;
    this.element.querySelector("select[name='q0']").value = this.poi.q[0];
    this.element.querySelector("input[name='q1']").value = this.poi.q[1];
    this.element.querySelector("input[name='q2']").value = this.poi.q[2];
    this.element.querySelector("input[name='q3']").value = this.poi.q[3];
    this.element.querySelector("input[name='qp']").value = this.poi.qp;
    this.element.querySelector("input[type='submit']").value = submitLabel;
  }
  
  onSubmitForm(event) {
    event.preventDefault();
    this.poi.x = +this.element.querySelector("input[name='x']").value;
    this.poi.y = +this.element.querySelector("input[name='y']").value;
    this.poi.q[0] = +this.element.querySelector("select[name='q0']").value;
    this.poi.q[1] = +this.element.querySelector("input[name='q1']").value;
    this.poi.q[2] = +this.element.querySelector("input[name='q2']").value;
    this.poi.q[3] = +this.element.querySelector("input[name='q3']").value;
    this.poi.qp = this.element.querySelector("input[name='qp']").value;
    this.onsubmit(this.poi);
    this.dom.popModal();
  }
}
