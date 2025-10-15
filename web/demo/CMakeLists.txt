cmake_minimum_required(VERSION 3.13)

project(Benchmark)

include(./../../third_party/vendor_tools/vendor.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif ()

set(SKIA_DIR ../../third_party/skia)
get_filename_component(SKIA_DIR "${SKIA_DIR}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

list(APPEND BENCH_INCLUDES ${SKIA_DIR})
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
list(APPEND BENCH_INCLUDES src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src)

file(GLOB_RECURSE BENCHMARK_FILES
        ../../src/base/*.*
        ../../src/benchmark/*.*
        ../../src/tools/*.*
        ../../src/window_context/*.*
        src/*.*)

if (DEFINED EMSCRIPTEN)
    add_executable(benchmark ${BENCHMARK_FILES})
    list(APPEND BENCHMARK_COMPILE_OPTIONS -fno-rtti -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0)
    list(APPEND BENCHMARK_LINK_OPTIONS --no-entry -lembind -fno-rtti -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0 -sEXPORT_NAME='Benchmark' -sWASM=1
            -sMAX_WEBGL_VERSION=2 -sEXPORTED_RUNTIME_METHODS=['GL','HEAPU8'] -sEXPORTED_FUNCTIONS=['_main'] -sMODULARIZE=1
            -sENVIRONMENT=web,worker -sEXPORT_ES6=1 -sASSERTIONS=2 -sDISABLE_EXCEPTION_CATCHING=0
            -sDISABLE_EXCEPTION_CATCHING -sNODEJS_CATCH_EXIT=0 -sDYNAMIC_EXECUTION=0 -sFORCE_FILESYSTEM=0 -sFILESYSTEM=0
    )
    set(unsupported_emsdk_versions "4.0.11")
    foreach (unsupported_version IN LISTS unsupported_emsdk_versions)
        if (${EMSCRIPTEN_VERSION} VERSION_EQUAL ${unsupported_version})
            message(FATAL_ERROR "Emscripten version ${EMSCRIPTEN_VERSION} is not supported.")
        endif ()
    endforeach ()
    if (EMSCRIPTEN_PTHREADS)
        list(APPEND BENCHMARK_LINK_OPTIONS -sUSE_PTHREADS=1 -sINITIAL_MEMORY=32MB -sALLOW_MEMORY_GROWTH=1
                -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency -sPROXY_TO_PTHREAD=1
                -sEXIT_RUNTIME=0 -sINVOKE_RUN=0 -sMALLOC=mimalloc)
        list(APPEND BENCHMARK_COMPILE_OPTIONS -fPIC -pthread)
        file(GLOB SKIA_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/out/skia/web/wasm-mt/libskia.a")
    else ()
        list(APPEND BENCHMARK_LINK_OPTIONS -sALLOW_MEMORY_GROWTH=1)
        file(GLOB SKIA_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/out/skia/web/wasm/libskia.a")
    endif ()
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND BENCHMARK_COMPILE_OPTIONS -O0 -g3)
        list(APPEND BENCHMARK_LINK_OPTIONS -O0 -g3 -sSAFE_HEAP=1 -Wno-limited-postlink-optimizations)
    else ()
        list(APPEND BENCHMARK_COMPILE_OPTIONS -Oz)
        list(APPEND BENCHMARK_LINK_OPTIONS -Oz)
    endif ()
else ()
    add_library(benchmark SHARED ${BENCHMARK_FILES})
endif ()

target_include_directories(benchmark PRIVATE ${BENCH_INCLUDES})
target_compile_options(benchmark PUBLIC ${BENCHMARK_COMPILE_OPTIONS})
target_link_options(benchmark PUBLIC ${BENCHMARK_LINK_OPTIONS})
target_link_libraries(benchmark PUBLIC ${SKIA_LIBRARIES})