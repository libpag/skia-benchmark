const fs = require('fs');
const path = require('path');

const EMSDK_COMMIT = "9dbdc4b3437750b85d16931c7c801bb71a782122";
const EMSDK_VERSION = "latest";

const ROOT_PATH = process.cwd();
const EMSDK_COMMIT_LINE = `"third_party/externals/emsdk"                  : "https://github.com/emscripten-core/emsdk.git@${EMSDK_COMMIT}",`;
const EMSDK_VERSION_LINE = `EMSDK_VERSION = '${EMSDK_VERSION}'`;

const DEPS_PATH = path.join(ROOT_PATH, 'DEPS');
const ACTIVATE_EMSDK_PATH = path.join(ROOT_PATH, 'bin', 'activate-emsdk');

function updateFile(filePath, regex, newLine) {
    try {
        let content = fs.readFileSync(filePath, 'utf8');
        if (regex.test(content)) {
            content = content.replace(regex, newLine);
        } else {
            content += `\n${newLine}\n`;
        }

        fs.writeFileSync(filePath, content, 'utf8');
    } catch (err) {
        console.error(`Error updating file ${filePath}:`, err);
    }
}

updateFile(DEPS_PATH, /"third_party\/externals\/emsdk".*/, EMSDK_COMMIT_LINE);
updateFile(ACTIVATE_EMSDK_PATH, /^EMSDK_VERSION =.*/m, EMSDK_VERSION_LINE);

/**
 * When compiling the wasm version on the Windows platform, the following modifications are
 * required. For details, refer to https://issues.skia.org/issues/40045374.
 */
if (process.platform === 'win32') {
    const BUILD_GN_PATH = path.join(ROOT_PATH, 'third_party', 'freetype2', 'BUILD.gn');
    try {
        let content = fs.readFileSync(BUILD_GN_PATH, 'utf8');
        content = content.replace(
            /<freetype-no-type1\/freetype\/config\/ftmodule.h>/g,
            '\\"freetype-no-type1/freetype/config/ftmodule.h\\"'
        ).replace(
            /<freetype-no-type1\/freetype\/config\/ftoption.h>/g,
            '\\"freetype-no-type1/freetype/config/ftoption.h\\"'
        );
        fs.writeFileSync(BUILD_GN_PATH, content, 'utf8');
        console.log('BUILD.gn file updated successfully!');
    } catch (err) {
        console.error(`Error updating BUILD.gn file: ${err.message}`);
    }
}