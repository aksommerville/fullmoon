import { Dom } from "../util/Dom.js";
import { Comm } from "../util/Comm.js";
import { ResourceService } from "../service/ResourceService.js";
import { DataFactory } from "../service/DataFactory.js";

export class HeaderController {
  static getDependencies() {
    return [HTMLElement, Dom, ResourceService, DataFactory, Comm, Window];
  }
  constructor(element, dom, resourceService, dataFactory, comm, window) {
    this.element = element;
    this.dom = dom;
    this.resourceService = resourceService;
    this.dataFactory = dataFactory;
    this.comm = comm;
    this.window = window;
    
    this.resourceStatusListener = this.resourceService.listenStatus((status) => this.onResourceStatus(status));
    
    this.buildUi();
    this.onResourceStatus(this.resourceService.status);
  }
  
  setResource(data) {
    const container = this.element.querySelector(".editorExtras");
    container.innerHTML = "";
    const controllerClass = this.dataFactory.extrasClassForData(data);
    if (!controllerClass) return;
    const controller = this.dom.spawnController(container, controllerClass);
    controller.setup(data);
  }
  
  onRemoveFromDom() {
    this.resourceService.unlistenStatus(this.resourceStatusListener);
  }
  
  buildUi() {
    this.element.innerHTML = "";
    this.dom.spawn(this.element, "DIV", ["syncStatus"]);
    this.dom.spawn(this.element, "BUTTON", ["saveButton"], "Save", { disabled: true, "on-click": () => this.onSave() });
    this.dom.spawn(this.element, "BUTTON", "Launch", { "on-click": () => this.onLaunch() });
    this.dom.spawn(this.element, "DIV", ["editorExtras"]);
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
  
  onLaunch() {
    this.comm.fetchBinary("POST", "/api/launch").then(output => {
    }).catch(message => {
      this.window.alert("Launch failed. See server logs for details.");
    });
  }
}
