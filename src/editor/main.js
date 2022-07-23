#!/usr/bin/env node
const http = require("http");
const urllib = require("url");

const cliparams = process.argv.filter(s => s.startsWith("--")).reduce((a, pair) => {
  const [k, v] = pair.substr(2).split('=');
  a[k] = v;
  return a;
}, {});

const host = cliparams.host || "localhost";
const port = +cliparams.port || 2040;

/* Servlets TOC.
 */
 
const serve_static = require("./serve_static.js");
const rest = require("./rest.js");
 
function findServlet(method, path) {
  if (path.startsWith("/api/")) {
    switch (method + path) {
      case "GET/api/reslist": return rest.getReslist;
      case "POST/api/launch": return rest.launch;
    }
  } else if (path.startsWith("/res/")) {
    return serve_static(method, "src/data", "/res");
  } else {
    if (method !== "GET") return null;
    return serve_static("GET", cliparams.htdocs);
  }
  return null;
}

/* Main bootstrap.
 */

const server = http.createServer((req, rsp) => {
  let body = "";
  req.on("data", data => body += data);
  req.on("end", () => {
    const url = urllib.parse(req.url);
    const query = url.query ? url.query.split('&').reduce((a, pair) => {
      const [k, v] = pair.split('=').map(decodeURIComponent);
      a[k] = v;
      return a;
    }, {}) : {};
    const servlet = findServlet(req.method, url.pathname);
    if (!servlet) {
      console.error(`Servlet not found: ${req.method} ${url.pathname}`);
      rsp.statusCode = 404;
      rsp.end("Not found\n");
    } else {
      servlet(rsp, req, url.pathname, query, body, url, cliparams).then(() => {
        if (!rsp.writableEnded) throw new Error(`Incomplete server response for ${req.method} ${url.pathname}`);
      }).catch((e) => {
        console.error(`${req.method} ${url.pathname}`, e);
        if ((typeof(e) === "number") && (e >= 400) && (e < 600)) {
          rsp.statusCode = e;
          switch (e) {
            case 404: rsp.statusMessage = "Not found"; break;
            case 405: rsp.statusMessage = "Method not allowed"; break;
            default: rsp.statusMessage = (e >= 500) ? "Internal server error" : "Bad request";
          }
        } else {
          rsp.statusCode = e.statusCode || 500;
          rsp.statusMessage = e.statusMessage || e.message || "Internal server error";
        }
        rsp.end();
      });
    }
  });
});

server.listen(port, host, () => {
  console.log(`listening on ${host}:${port}`);
});
