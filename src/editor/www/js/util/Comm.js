export class Comm {
  static getDependencies() {
    return [Window];
  }
  constructor(window) {
    this.window = window;
  }
  
  fetchJson(method, path, query, body) {
    return this.window.fetch(
      this.composeHttpPath(path, query),
      { method, body },
    ).then(response => {
      if (!response.ok) throw response;
      return response.json();
    });
  }
  
  fetchBinary(method, path, query, body) {
    return this.window.fetch(
      this.composeHttpPath(path, query),
      { method, body },
    ).then(response => {
      if (!response.ok) throw response;
      return response.arrayBuffer();
    });
  }
  
  composeHttpPath(path, query) {
    path = `${this.window.location.protocol}//${this.window.location.host}${path}`;
    if (query) {
      let separator = "?";
      for (const k of Object.keys(query)) {
        path += `${separator}${encodeURIComponent(k)}=${encodeURIComponent(query[k])}`;
        separator = "&";
      }
    }
    return path;
  }
}

Comm.singleton = true;
