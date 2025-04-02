#!/bin/bash

ROOT_PATH=$(pwd)

EMSDK_COMMIT="9dbdc4b3437750b85d16931c7c801bb71a782122"
EMSDK_VERSION="latest"

EMSDK_COMMIT_LINE="  \"third_party/externals/emsdk\"                  : \"https://skia.googlesource.com/external/github.com/emscripten-core/emsdk.git@${EMSDK_COMMIT}\","
EMSDK_VERSION_LINE="EMSDK_VERSION = '${EMSDK_VERSION}'"

sed -i '' '/"third_party\/externals\/emsdk"/{
  a\
'"$EMSDK_COMMIT_LINE"'
  d
}' "${ROOT_PATH}"/third_party/skia/DEPS

sed -i '' '/^EMSDK_VERSION =/{
  a\
'"$EMSDK_VERSION_LINE"'
  d
}' "${ROOT_PATH}"/third_party/skia/bin/activate-emsdk
