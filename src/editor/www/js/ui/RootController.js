import { Dom } from "../util/Dom.js";
import { HeaderController } from "./HeaderController.js";
import { ToolboxController } from "./ToolboxController.js";
import { CanvasController } from "./CanvasController.js";
import { ResourceService } from "../service/ResourceService.js";

export class RootController {
  static getDependencies() {
    return [HTMLElement, Dom, Window, ResourceService];
  }
  constructor(element, dom, window, resourceService) {
    this.element = element;
    this.dom = dom;
    this.window = window;
    this.resourceService = resourceService;
    
    this.headerController = null;
    this.toolboxController = null;
    this.canvasController = null;
    
    this.buildUi();
  }
  
  buildUi() {
    this.element.innerHTML = "";
    
    this.headerController = this.dom.spawnController(this.element, HeaderController);
    
    const midRow = this.dom.spawn(this.element, "DIV", ["midRow"]);
    
    this.toolboxController = this.dom.spawnController(midRow, ToolboxController);
    this.toolboxController.oneditresource = (path, displayName) => this.onEditResource(path, displayName);
    this.toolboxController.onnewmap = () => this.onNewMap();
    
    this.canvasController = this.dom.spawnController(midRow, CanvasController);
  }
  
  onEditResource(path, displayName) {
    this.canvasController.setResource(path, displayName);
  }
  
  onNewMap() {
    const name = this.window.prompt("Name for new map:", `${Date.now()}`);
    if (!name) return;
    const path = "/res/map/" + name;
    this.resourceService.dirty(path, () => "").then(() => {
      return this.toolboxController.refreshResourcePaths();
    }).then(() => {
      this.onEditResource(path, name);
    });
    this.resourceService.saveAll();
  }
}
