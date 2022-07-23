const fs = require("fs").promises;
const pathlib = require("path");
const child_process = require("child_process");

function recursiveReaddir(path, removePrefix, output) {
  return fs.readdir(path, { withFileTypes: true }).then((files) => {
    const promises = [];
    for (const file of files) {
      if (file.isDirectory()) {
        promises.push(recursiveReaddir(pathlib.join(path, file.name), removePrefix, output));
      } else if (file.isFile()) {
        let subpath = pathlib.join(path, file.name);
        if (subpath.startsWith(removePrefix)) {
          subpath = "/res" + subpath.substr(removePrefix.length);
        }
        output.push(subpath);
      }
    }
    return Promise.all(promises);
  });
}

function getReslist(rsp, req, path, query, body, url, cliparams) {
  const output = [];
  return recursiveReaddir("src/data", "src/data", output).then(() => {
    rsp.setHeader("Content-Type", "application/json");
    rsp.end(JSON.stringify(output));
  });
}

function launch(rsp, req, path, query, body, url, cliparams) {
  return new Promise((resolve, reject) => {
    const process = child_process.exec("make run", (error, stdout, stderr) => {
      if (error) {
        rsp.statusCode = 500;
        console.error(error);
        rsp.end();
      } else {
        rsp.setHeader("Content-Type", "text/plain");
        rsp.end(stdout + "\n" + stderr);
      }
      resolve();
    });
  });
}

module.exports = {
  getReslist,
  launch,
};
