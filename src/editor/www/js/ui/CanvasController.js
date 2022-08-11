import { Dom } from "../util/Dom.js";
import { ResourceService } from "../service/ResourceService.js";
import { DataFactory } from "../service/DataFactory.js";

export class CanvasController {
  static getDependencies() {
    return [HTMLElement, Dom, ResourceService, DataFactory];
  }
  constructor(element, dom, resourceService, dataFactory) {
    this.element = element;
    this.dom = dom;
    this.resourceService = resourceService;
    this.dataFactory = dataFactory;
    
    this.dataController = null;
    
    this.buildUi();
  }
  
  buildUi() {
    this.element.innerHTML = "";
  }
  
  setResource(path, displayName, extra) {
    return this.resourceService.getResource(path).then((content) => {
      const data = this.dataFactory.decode(content, path);
      this.populate(data, extra);
      return data;
    }).catch((e) => {
      console.log(`CanvasController failed to load resource '${displayName}' (${path})`, e);
      return null;
    });
  }
  
  populate(data, extra) {
    this.element.innerHTML = "";
    if (!data) return;
    const controllerClass = this.dataFactory.controllerClassForData(data);
    if (!controllerClass) throw new Error(`No editor controller class`);
    this.dataController = this.dom.spawnController(this.element, controllerClass);
    this.dataController.setup(data, extra);
  }
}
