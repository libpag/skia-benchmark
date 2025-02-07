#!/bin/bash -e

ROOT_PATH=$(dirname $(dirname $(dirname "$0")))

. "${ROOT_PATH}"/third_party/vendor_tools/Tools.sh
python3 "${ROOT_PATH}"/third_party/skia/tools/git-sync-deps

buildArch() {
  arch=$1
  OUT_REAL_PATH=out/mac/${arch}
  bin/gn gen "${OUT_REAL_PATH}" --args="is_official_build=true
                                  target_cpu=\"${arch}\"
                                  is_debug=false
                                  skia_use_system_libjpeg_turbo=false
                                  skia_use_system_libpng=false
                                  skia_use_system_libwebp=false
                                  skia_use_system_harfbuzz=false
                                  skia_use_system_icu=false
                                  skia_use_system_expat=false
                                  skia_use_system_zlib=false"

  ninja -C "${OUT_REAL_PATH}"

  SOURCE_DIR=$(pwd)
  BUILD_DIR=${SOURCE_DIR}/${OUT_REAL_PATH}

  copy_library -p mac -l "${arch}=${BUILD_DIR}/*.a"
  #rm -rf ${SOURCE_DIR}/out
}

IFS=',' read -ra ARCH_ARRAY <<< "$VENDOR_ARCHS"
for arch in "${ARCH_ARRAY[@]}"; do
  buildArch "$arch"
done





