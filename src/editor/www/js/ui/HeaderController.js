import { Dom } from "../util/Dom.js";
import { ResourceService } from "../service/ResourceService.js";

export class HeaderController {
  static getDependencies() {
    return [HTMLElement, Dom, ResourceService];
  }
  constructor(element, dom, resourceService) {
    this.element = element;
    this.dom = dom;
    this.resourceService = resourceService;
    
    this.resourceStatusListener = this.resourceService.listenStatus((status) => this.onResourceStatus(status));
    
    this.buildUi();
    this.onResourceStatus(this.resourceService.status);
  }
  
  onRemoveFromDom() {
    this.resourceService.unlistenStatus(this.resourceStatusListener);
  }
  
  buildUi() {
    this.element.innerHTML = "";
    this.dom.spawn(this.element, "DIV", ["syncStatus"]);
    this.dom.spawn(this.element, "BUTTON", ["saveButton"], "Save", { disabled: true, "on-click": () => this.onSave() });
  }
  
  onResourceStatus(status) {
    let element;
    if (element = this.element.querySelector(".syncStatus")) {
      element.classList.remove("dirty");
      element.classList.remove("clean");
      element.classList.remove("error");
      element.classList.add(status);
    }
    if (element = this.element.querySelector(".saveButton")) {
      if ((status === "clean") || !this.resourceService.hasPendingChanges()) {
        element.setAttribute("disabled", "true");
      } else {
        element.removeAttribute("disabled");
      }
    }
  }
  
  onSave() {
    this.resourceService.saveAll();
  }
}
