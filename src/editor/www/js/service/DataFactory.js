/* DataFactory.js
 * Generic interface to the various model objects we can work with, and their UI controllers.
 */
 
export class DataFactory {
  static getDependencies() {
    return [];
  }
  constructor() {
  }
  
  decode(serial, path) {
    return null;//TODO
  }
  
  controllerClassForData(data) {
    return null;//TODO
  }
}

DataFactory.singleton = true;
