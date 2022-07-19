import { Injector } from "./js/util/Injector.js";
import { Dom } from "./js/util/Dom.js";
import { RootController } from "./js/ui/RootController.js";

window.addEventListener("load", () => {
  const injector = new Injector(window, document);
  const dom = injector.getInstance(Dom);
  
  document.body.innerHTML = "";
  dom.spawnController(document.body, RootController);
});
