import { Dom } from "../util/Dom.js";
import { ResourceService } from "../service/ResourceService.js";

export class ToolboxController {
  static getDependencies() {
    return [HTMLElement, Dom, ResourceService];
  }
  constructor(element, dom, resourceService) {
    this.element = element;
    this.dom = dom;
    this.resourceService = resourceService;
    
    // Owner should assign directly:
    this.oneditresource = (path, displayName) => {};
    this.onnewmap = () => {};
    
    this.buildUi();
    
    this.resourceService.getResourcePaths().then(paths => this.onGetResourcePaths(paths));
  }
  
  refreshResourcePaths() {
    return this.resourceService.flushResourcePaths().then(paths => this.onGetResourcePaths(paths));
  }
  
  buildUi() {
    this.element.innerHTML = "";
    
    const actions = this.dom.spawn(this.element, "DIV", ["actions"]);
    this.dom.spawn(actions, "INPUT", { type: "button", value: "New Map", "on-click": () => this.onnewmap() });
    
    this.dom.spawn(this.element, "UL", ["reslist"]);
  }
  
  onGetResourcePaths(paths) {
    const reslist = this.element.querySelector(".reslist");
    reslist.innerHTML = "";
    for (let path of paths) {
      const displayName = path.replace(/^.*\/([^\/]*)$/, "$1");
      this.dom.spawn(reslist, "LI", ["res"], displayName, { "on-click": () => this.oneditresource(path, displayName) });
    }
  }
}
