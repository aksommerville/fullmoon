/* ResourceService.js
 * Manages access to resources (src/data/*), and related state.
 */

import { Comm } from "../util/Comm.js";

export class ResourceService {
  static getDependencies() {
    return [Comm, Window];
  }
  constructor(comm, window) {
    this.comm = comm;
    this.window = window;
    
    this.statusListeners = [];
    this.nextStatusListenerId = 1;
    this.status = "clean";
    
    this.pendingChanges = {}; // path:{encode,resolve,reject}
    this.dirtyTimer = null;
    this.dirtyTime = 5000;
    
    this.resourcePaths = []; // Just the HTTP path eg "/res/image/titlesplash.png", as reported by GET /api/reslist
    this.resourcePathsUpdating = null; // null or Promise
  }
  
  getResource(path) {
    // We could check pendingChanges and if present there, sync first.
    // I think not worth it.
    if (!path.startsWith("/res/")) return Promise.reject(`Invalid resource path '${path}'`);
    return this.comm.fetchBinary("GET", path);
  }
  
  /* TOC.
   ********************************************************************/
   
  getResourcePaths() {
    if (this.resourcePathsUpdating) return this.resourcePathsUpdating;
    if (this.resourcePaths.length) return Promise.resolve(this.resourcePaths);
    return this.resourcePathsUpdating = this.comm.fetchJson("GET", "/api/reslist").catch((e) => {
      console.error(`Failed to fetch resource list`, e);
      return [];
    }).then((list) => {
      this.resourcePaths = list;
      this.resourcePathsUpdating = null;
      return list;
    });
  }
  
  flushResourcePaths() {
    this.resourcePaths = [];
    if (this.resourcePathsUpdating) return;
    return this.getResourcePaths();
  }
  
  /* Save content and report dirty content.
   ********************************************************************/
   
  saveAll() {
    if (this.dirtyTimer) {
      this.window.clearTimeout(this.dirtyTimer);
      this.dirtyTimer = null;
    }
    this._syncChanges();
  }
  
  hasPendingChanges() {
    return Object.keys(this.pendingChanges).length > 0;
  }
  
  dirty(name, encode) {
    const prior = this.pendingChanges[name];
    if (prior) prior[1]("replaced");
    let resolve, reject;
    const promise = new Promise((rs, rj) => {
      resolve = rs;
      reject = rj;
    });
    this.pendingChanges[name] = {encode, resolve, reject};
    this._resetDirtyTimer();
    return promise;
  }
  
  _resetDirtyTimer() {
    this._setStatus("dirty");
    if (this.dirtyTimer) {
      this.window.clearTimeout(this.dirtyTimer);
    }
    this.dirtyTimer = this.window.setTimeout(() => {
      this.dirtyTimer = null;
      this._syncChanges();
    }, this.dirtyTime);
  }
  
  _syncChanges() {
    const changes = this.pendingChanges;
    this.pendingChanges = {};
    let nextStatus = "clean";
    for (const path of Object.keys(changes)) {
      const { encode, resolve, reject } = changes[path];
      try {
        const data = encode();
        this.comm.fetchBinary("PUT", path, null, data).then(resolve).catch((e) => {
          // Asynchronous error during upload.
          this._setStatus("error");
          reject(e);
        });
      } catch (e) {
        // Synchronous error during encode.
        reject(e);
        nextStatus = "error";
      }
    }
    this._setStatus(nextStatus);
  }
  
  /* Status: Are any changes pending, and has a save call failed?
   *********************************************************************/
  
  listenStatus(cb) {
    const id = this.nextStatusListenerId++;
    this.statusListeners.push({ cb, id });
    return id;
  }
  
  unlistenStatus(id) {
    const p = this.statusListeners.findIndex(l => l.id === id);
    if (p < 0) return;
    this.statusListeners.splice(p, 1);
  }
  
  _setStatus(newStatus) {
    if (newStatus === this.status) return;
    if (!this._validStatus(newStatus)) throw new Error(`not a valid Resource status: '${newStatus}'`);
    this.status = newStatus;
    for (const { cb } of this.statusListeners) cb(this.status);
  }
  
  _validStatus(status) {
    switch (status) {
      case "clean":
      case "dirty":
      case "error":
        return true;
    }
    return false;
  }
}

ResourceService.singleton = true;
