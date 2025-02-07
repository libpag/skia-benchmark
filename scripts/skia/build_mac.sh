#!/bin/bash -e

ROOT_PATH=$(dirname $(dirname $(dirname "$0")))

. "${ROOT_PATH}"/third_party/vendor_tools/Tools.sh
python3 "${ROOT_PATH}"/third_party/skia/tools/git-sync-deps

IFS=',' read -ra ARCH_ARRAY <<< "$VENDOR_ARCHS"

arch=""
for element in "${ARCH_ARRAY[@]}"; do
    arch=$element
    break # Just use the first arch
done

# If no arch is specified, use the local machine's arch
if [[ -z $arch ]]; then
  local=$(uname -m)
  if [[ "$local" == "x86_64" ]]; then
    arch="x64"
  else
    arch="arm64"
  fi
fi


OUT_REAL_PATH=out/mac/${arch}
bin/gn gen ${OUT_REAL_PATH} --args="is_official_build=true
                                target_cpu=\"${arch}\"
                                is_debug=false
                                skia_use_system_libjpeg_turbo=false
                                skia_use_system_libpng=false
                                skia_use_system_libwebp=false
                                skia_use_system_harfbuzz=false
                                skia_use_system_icu=false
                                skia_use_system_expat=false
                                skia_use_system_zlib=false"

ninja -C ${OUT_REAL_PATH}

SOURCE_DIR=$(pwd)
BUILD_DIR=${SOURCE_DIR}/${OUT_REAL_PATH}

copy_library -p mac -l "${arch}=${BUILD_DIR}/*.a"
#rm -rf ${SOURCE_DIR}/out

