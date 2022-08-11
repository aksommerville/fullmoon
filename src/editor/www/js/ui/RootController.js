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
    
    this.resourceService.oneditresource = (path, name, extra) => this.onEditResource(path, name, extra);
    
    this.buildUi();
  }
  
  onRemoveFromDom() {
    this.resourceService.oneditResource = () => {};
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
  
  onEditResource(path, displayName, extra) {
    this.canvasController.setResource(path, displayName, extra).then((data) => {
      this.toolboxController.setResource(data);
      this.headerController.setResource(data);
    });
  }
  
  onNewMap() {
    const name = this.window.prompt("Name for new map:", `m${Date.now() % 10000}`);
    if (!name) return;
    const path = "/res/map/" + name + ".txt";
    const row = "000000000000000000\n";
    const content = "tilesheet=bgtiles\nsize=9 5\nBEGIN_BODY\n" + row + row + row +row + row;
    this.resourceService.dirty(path, () => content).then(() => {
      return this.toolboxController.refreshResourcePaths();
    }).then(() => {
      this.onEditResource(path, name);
    });
    this.resourceService.saveAll();
  }
}
