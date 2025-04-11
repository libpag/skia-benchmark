const path = require('path');
const fs = require('fs');
const process = require("process");
const Utils = require("../../third_party/vendor_tools/lib/Utils");

const ROOT_PATH = path.resolve(__dirname, '../..');
const isWin = process.platform === 'win32';
const skiaPath = path.join(ROOT_PATH, 'third_party', 'skia');
const pythonCmd = isWin ? "python" : "python3";
Utils.exec(`${pythonCmd} tools/git-sync-deps`, skiaPath);
Utils.exec(`${pythonCmd} bin/fetch-ninja`, skiaPath);

function buildArch(arch) {
    let OUT_REAL_PATH = path.join(ROOT_PATH, 'third_party', 'out', 'skia', 'web', arch);
    if (!fs.existsSync(OUT_REAL_PATH)) {
        fs.mkdirSync(OUT_REAL_PATH, { recursive: true });
    }
    const args = [
        'is_official_build=true',
        'is_debug=false',
        'target_cpu=\\"wasm\\"',
        'is_component_build=false',
        'werror=true',
        'skia_use_angle=false',
        'skia_use_dng_sdk=false',
        'skia_use_dawn=false',
        'skia_use_webgl=true',
        'skia_use_webgpu=false',
        'skia_use_expat=false',
        'skia_use_fontconfig=false',
        'skia_use_freetype=true',
        'skia_use_libheif=false',
        'skia_use_libjpeg_turbo_decode=true',
        'skia_use_libjpeg_turbo_encode=true',
        'skia_use_no_jpeg_encode=false',
        'skia_use_libpng_decode=true',
        'skia_use_libpng_encode=true',
        'skia_use_no_png_encode=false',
        'skia_use_libwebp_decode=true',
        'skia_use_libwebp_encode=true',
        'skia_use_no_webp_encode=false',
        'skia_use_lua=false',
        'skia_use_piex=false',
        'skia_use_system_freetype2=false',
        'skia_use_system_libjpeg_turbo=false',
        'skia_use_system_libpng=false',
        'skia_use_system_libwebp=false',
        'skia_use_system_zlib=false',
        'skia_use_vulkan=false',
        'skia_use_wuffs=true',
        'skia_use_zlib=true',
        'skia_enable_ganesh=true',
        'skia_enable_graphite=false',
        'skia_build_for_debugger=false',
        'skia_enable_skottie=true',
        'skia_use_icu=true',
        'skia_use_client_icu=false',
        'skia_use_libgrapheme=false',
        'skia_use_icu4x=false',
        'skia_use_system_icu=false',
        'skia_use_harfbuzz=true',
        'skia_use_system_harfbuzz=false',
        'skia_enable_fontmgr_custom_embedded=true',
        'skia_enable_fontmgr_custom_empty=true',
        'skia_use_freetype_woff2=true',
        'skia_enable_fontmgr_custom_directory=true',
        'skia_enable_skshaper=true',
        'skia_enable_skparagraph=true',
        'skia_enable_pdf=false',
        'skia_canvaskit_enable_rt_shader=true',
        'skia_canvaskit_force_tracing=true',
        'skia_canvaskit_profile_build=false',
        'skia_canvaskit_enable_skp_serialization=true',
        'skia_canvaskit_enable_effects_deserialization=true',
        'skia_canvaskit_include_viewer=false',
        'skia_canvaskit_enable_pathops=true',
        'skia_canvaskit_enable_matrix_helper=true',
        'skia_canvaskit_enable_canvas_bindings=true',
        'skia_canvaskit_enable_font=true',
        'skia_canvaskit_enable_embedded_font=false',
        'skia_canvaskit_enable_alias_font=true',
        'skia_canvaskit_legacy_draw_vertices_blend_mode=false',
        'skia_canvaskit_enable_debugger=false',
        'skia_canvaskit_enable_paragraph=true',
        'skia_canvaskit_enable_webgl=true',
        'skia_canvaskit_enable_webgpu=false',
    ];
    if (arch === 'wasm-mt') {
        args.push('extra_cflags=[\\"-pthread\\"]');
    }

    const filteredArgs = args.filter(arg => arg !== '');
    const argString = filteredArgs.join(' ');
    const gnCmd = path.join(skiaPath, 'bin', 'gn');
    let cmd = `${gnCmd} gen ${OUT_REAL_PATH} --args="${argString}"`;
    Utils.exec(cmd, skiaPath);

    const ninjaPath = path.join(skiaPath, 'third_party', 'ninja', 'ninja');
    const ninjaCmd = `${ninjaPath} -C "${OUT_REAL_PATH}"`
    Utils.exec(ninjaCmd, skiaPath);
}

const VENDOR_ARCHS = process.env.VENDOR_ARCHS || 'wasm,wasm-mt';
const ARCH_ARRAY = VENDOR_ARCHS.split(',');

for (const arch of ARCH_ARRAY) {
    buildArch(arch);
}