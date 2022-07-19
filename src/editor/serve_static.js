const pathlib = require("path");
const fs = require("fs").promises;

/* Content-Type.
 */
 
function guessContentType(path, content) {
  const sfx = (path.replace(/^.*\.([^\/.]*)$/, "$1") || "").toLowerCase();
  
  // If we know it from the suffix, that's the final word.
  switch (sfx) {
    case "html": return "text/html";
    case "png": return "image/png";
    case "css": return "text/css";
    case "js": return "application/javascript";
    case "txt": return "text/plain";
    case "ico": return "image/x-icon";
  }
  
  // Empty files are binary, that's debatable of course.
  if (!content || !content.length) return "application/octet-stream";
  
  // Expect binary files to have a C0 or UTF8-illegal byte in the first 16.
  let i = Math.min(16, content.length);
  while (i-->0) {
    const ch = content[i];
    // A wee exception: LF, HT, and CR are "text".
    if (ch === 0x0a) continue;
    if (ch === 0x09) continue;
    if (ch === 0x0d) continue;
    if (ch < 0x20) return "application/octet-stream";
    if (ch >= 0xf8) return "application/octet-stream";
  }
  return "text/plain";
}

/* GET.
 */
 
function getFile(rsp, path) {
  return fs.readFile(path).then((content) => {
    rsp.setHeader("Content-Type", guessContentType(path, content));
    rsp.end(content);
  });
}

/* PUT.
 */
 
function putFile(rsp, path, body) {
  return fs.writeFile(path, body).then(() => {
    rsp.end();
  });
}

/* DELETE.
 */
 
function deleteFile(rsp, path) {
  return fs.unlink(path).then(() => {
    rsp.end();
  });
}

/* Main dispatch.
 */

module.exports = (method, htdocs, removePrefix) => {
  if (htdocs) return (rsp, req, rpath, query, body, url, cliparams) => {
    try {
      if (removePrefix && rpath.startsWith(removePrefix)) {
        rpath = rpath.substr(removePrefix.length);
      }
      if (rpath === "/") rpath = "/index.html";
      const path = pathlib.normalize(pathlib.join(htdocs, rpath));
      if (!path.startsWith(htdocs)) throw 404;
      switch (method) {
        case "GET": return getFile(rsp, path);
        case "PUT": return putFile(rsp, path, body || "");
        case "DELETE": return deleteFile(rsp, path);
      }
      throw 405;
    } catch (e) {
      return Promise.reject(e);
    }
  };
  return null;
};
