#!/bin/bash -e

ROOT_PATH=$(dirname $(dirname $(dirname "$0")))

. "${ROOT_PATH}"/third_party/vendor_tools/Tools.sh
python3 "${ROOT_PATH}"/third_party/skia/tools/git-sync-deps

function make_dir() {
  rm -rf $1
  mkdir -p $1
}

buildArch() {
  arch=$1
  OUT_REAL_PATH=out/mac/${arch}
  make_dir ${OUT_REAL_PATH}
  bin/gn gen "${OUT_REAL_PATH}" --args="target_cpu=\"${arch}\"
                                  is_official_build=true
                                  is_debug=false
                                  skia_enable_pdf=false
                                  skia_enable_skottie=false
                                  skia_enable_skparagraph=false
                                  skia_enable_skshaper=false
                                  skia_enable_skshaper_tests=false
                                  skia_enable_svg=false
                                  skia_enable_tools=false
                                  skia_use_system_libjpeg_turbo=false
                                  skia_use_system_libpng=false
                                  skia_use_system_libwebp=false
                                  skia_use_system_expat=false
                                  skia_use_system_zlib=false"

  ninja -C "${OUT_REAL_PATH}"

  SOURCE_DIR=$(pwd)
  BUILD_DIR=${SOURCE_DIR}/${OUT_REAL_PATH}

  copy_library -p mac -l "${arch}=${BUILD_DIR}/*.a"
  rm -rf ${BUILD_DIR}
}

IFS=',' read -ra ARCH_ARRAY <<< "$VENDOR_ARCHS"
for arch in "${ARCH_ARRAY[@]}"; do
  buildArch "$arch"
done





