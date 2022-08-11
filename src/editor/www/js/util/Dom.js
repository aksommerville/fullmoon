import { Injector } from "./Injector.js";

export class Dom {
  static getDependencies() {
    return [Injector, Window, Document];
  }
  constructor(injector, window, document) {
    this.injector = injector;
    this.window = window;
    this.document = document;
    
    this.mutationObserver = new this.window.MutationObserver(events => this.onMutation(events));
    this.mutationObserver.observe(this.document.body, { subtree: true, childList: true });
  }
  
  /* (args) may contain:
   *  - Array of strings: CSS class names.
   *  - String: innerText.
   *  - Object:
   *  - - "on-*": event listener.
   *  - - all else: HTML attribute.
   */
  spawn(parent, tagName, ...args) {
    const element = this.document.createElement(tagName);
    for (const arg of args) {
    
      if (arg instanceof Array) {
        for (const cls of arg) element.classList.add(cls);
    
      } else if (typeof(arg) === "string") {
        element.innerText = arg;
    
      } else if (arg && (typeof(arg) === "object")) {
        for (const k of Object.keys(arg)) {
          if (k.startsWith("on-")) {
            element.addEventListener(k.substr(3), arg[k]);
          } else {
            element.setAttribute(k, arg[k]);
          }
        }
        
      } else {
        throw new Error(`unexpected argument to Dom.spawn(): ${JSON.stringify(arg)}`);
      } 
    }
    parent.appendChild(element);
    return element;
  }
  
  /* Appends an element to (parent) and attaches a new controller instance to it.
   */
  spawnController(parent, clazz) {
    const tag = this.tagNameForControllerClass(clazz);
    const element = this.spawn(parent, tag, [clazz.name]);
    try {
      const controller = this.injector.getInstance(clazz, [element]);
      element.__gp1_controller = controller;
      return controller;
    } catch (e) {
      element.remove();
      throw e;
    }
  }
  
  spawnModal() {
    const layer = this.requireModalLayer();
    const blotter = this.requireModalBlotter(layer);
    blotter.parentNode.insertBefore(blotter, null);
    const wrapper = this.spawn(layer, "DIV", ["modalWrapper"]);
    layer.style.pointerEvents = "auto";
    return wrapper;
  }
  
  popModal(wrapperOrNone) {
    const layer = this.document.querySelector(".modalLayer");
    if (!layer) return;
    let topModal = null, nextTopModal = null;
    for (const wrapper of layer.querySelectorAll(".modalWrapper")) {
      nextTopModal = topModal;
      topModal = wrapper;
    }
    if (!topModal) return;
    if (wrapperOrNone && (wrapperOrNone !== topModal)) {
      wrapperOrNone.remove();
    } else {
      topModal.remove();
      const blotter = layer.querySelector(".modalBlotter");
      if (nextTopModal) {
        layer.insertBefore(blotter, nextTopModal);
      } else {
        blotter.remove();
        layer.style.pointerEvents = "none";
      }
    }
  }
  
  hasModal() {
    return !!this.document.querySelector(".modalWrapper");
  }
  
  /* private **********************************************************************/
  
  tagNameForControllerClass(clazz) {
    if (!clazz.getDependencies) return "DIV";
    for (const depClass of clazz.getDependencies()) {
      const match = depClass.name?.match(/^HTML(.*)Element$/);
      if (!match) continue;
      
      switch (match[1]) {
        case "": return "DIV";
        case "UList": return "UL";
      }
      
      return match[1].toUpperCase();
    }
    return "DIV";
  }
  
  onMutation(events) {
    for (const event of events) {
      if (event.removedNodes) {
        for (const element of event.removedNodes) {
          const controller = element.__gp1_controller;
          if (!controller) continue;
          delete element.__gp1_controller;
          if (controller.onRemoveFromDom) controller.onRemoveFromDom();
        }
      }
    }
  }
  
  requireModalLayer() {
    let layer = this.document.querySelector(".modalLayer");
    if (!layer) {
      layer = this.spawn(this.document.body, "DIV", ["modalLayer"]);
    }
    return layer;
  }
  
  requireModalBlotter(layer) {
    let blotter = this.document.querySelector(".modalBlotter");
    if (!blotter) {
      blotter = this.spawn(layer, "DIV", ["modalBlotter"], { "on-click": (event) => this.onModalBlotterClick(event) });
    }
    return blotter;
  }
  
  onModalBlotterClick(event) {
    event.stopPropagation();
    this.popModal();
  }
}

Dom.singleton = true;
