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