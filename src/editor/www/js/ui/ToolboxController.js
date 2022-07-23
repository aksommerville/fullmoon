import { Dom } from "../util/Dom.js";
import { ResourceService } from "../service/ResourceService.js";
import { DataFactory } from "../service/DataFactory.js";

export class ToolboxController {
  static getDependencies() {
    return [HTMLElement, Dom, ResourceService, DataFactory];
  }
  constructor(element, dom, resourceService, dataFactory) {
    this.element = element;
    this.dom = dom;
    this.resourceService = resourceService;
    this.dataFactory = dataFactory;
    
    // Owner should assign directly:
    this.oneditresource = (path, displayName) => {};
    this.onnewmap = () => {};
    
    this.buildUi();
    
    this.resourceService.getResourcePaths()
      .then(paths => this.filterResources(paths))
      .then(paths => this.onGetResourcePaths(paths));
  }
  
  setResource(data) {
    const accordion = this.element.querySelector(".tools");
    this.clearAccordion(accordion);
    const controllerClass = this.dataFactory.toolboxClassForData(data);
    if (!controllerClass) return;
    const li = this.dom.spawn(accordion, "LI");
    const controller = this.dom.spawnController(li, controllerClass);
    controller.setup(data);
  }
  
  refreshResourcePaths() {
    return this.resourceService.flushResourcePaths()
      .then(paths => this.filterResources(paths))
      .then(paths => this.onGetResourcePaths(paths));
  }
  
  filterResources(paths) {
    return paths.filter(path => {
      // Images are not interesting, nothing we can do with them.
      if (path.endsWith(".png")) return false;
      return true;
    }).sort();
  }
  
  buildUi() {
    this.element.innerHTML = "";
    
    const actions = this.dom.spawn(this.element, "DIV", ["actions"]);
    this.dom.spawn(actions, "INPUT", { type: "button", value: "New Map", "on-click": () => this.onnewmap() });
    
    const reslist = this.dom.spawn(this.element, "UL", ["reslist", "accordion", "open"]);
    this.dom.spawn(reslist, "DIV", ["accordion-header"], "Resources", { "on-click": (event) => this.onClickAccordion(event) });
    
    const tools = this.dom.spawn(this.element, "UL", ["tools", "accordion", "open"]);
    this.dom.spawn(tools, "DIV", ["accordion-header"], "Tools", { "on-click": (event) => this.onClickAccordion(event) });
  }
  
  clearAccordion(parent) {
    for (let i = parent.children.length; i-->0; ) {
      const child = parent.children[i];
      if (child.tagName === "LI") child.remove();
    }
  }
  
  onGetResourcePaths(paths) {
    const reslist = this.element.querySelector(".reslist");
    this.clearAccordion(reslist);
    for (let path of paths) {
      const displayName = this.displayNameForResourcePath(path);
      const li = this.dom.spawn(reslist, "LI", ["res"], { "on-click": () => this.oneditresource(path, displayName) });
      this.dom.spawn(li, "IMG", { src: `/img/res-${this.resourceTypeTagFromPath(path)}.png` });
      this.dom.spawn(li, "SPAN", displayName);
    }
  }
  
  displayNameForResourcePath(path) {
    const base = path.replace(/^.*\//, "");
    return base.replace(/_props.txt$/, "").replace(/\..*$/, "");
  }
  
  resourceTypeTagFromPath(path) {
    if (path.endsWith("_props.txt")) return "tilesheet";
    if (path.startsWith("/res/map/")) return "map";
    return "unknown";
  }
  
  onClickAccordion(event) {
    const accordion = this.findAccordionAncestor(event.target);
    if (!accordion) return;
    if (accordion.classList.contains("open")) {
      accordion.classList.remove("open");
      accordion.classList.add("closed");
    } else {
      accordion.classList.remove("closed");
      accordion.classList.add("open");
    }
  }
  
  findAccordionAncestor(element) {
    while (element) {
      if (element.classList.contains("accordion")) break;
      element = element.parentNode;
    }
    return element;
  }
}
