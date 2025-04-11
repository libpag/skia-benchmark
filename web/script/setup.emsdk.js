const path = require('path');
const fs = require('fs');
const process = require("process");
const Utils = require("../../third_party/vendor_tools/lib/Utils");

const emsdkPath = path.resolve(__dirname, '../../third_party/skia/third_party/externals/emsdk');
if (!fs.existsSync(emsdkPath)) {
    const skiaPath = path.resolve(__dirname, '../../third_party/skia');
    const isWin = process.platform === 'win32';
    const pythonCmd = isWin ? "python" : "python3";
    Utils.exec(`${pythonCmd} tools/git-sync-deps`, skiaPath);
}

const emscriptenPath = path.resolve(emsdkPath, 'upstream/emscripten');
process.env.PATH = process.platform === 'win32'
    ? `${emsdkPath};${emscriptenPath};${process.env.PATH}`
    : `${emsdkPath}:${emscriptenPath}:${process.env.PATH}`;

const emsdkEnv = process.platform === 'win32' ? "emsdk_env.bat" : "source emsdk_env.sh";
let result = Utils.execSafe(emsdkEnv, emsdkPath);
let lines = result.split("\n");
for (let line of lines) {
    let values = line.split("=");
    if (values.length > 1) {
        process.stdout.write(line);
        let key = values[0].trim();
        process.env[key] = values[1].trim();
    }
}