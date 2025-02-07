cmake_minimum_required(VERSION 3.13)
project(Benchmark)

set(COMP_TARGET Benchmark)

#set(CMAKE_VERBOSE_MAKEFILE ON)
include(./third_party/vendor_tools/vendor.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0063 NEW)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  list(APPEND BENCH_COMPILE_OPTIONS -Werror -Wall -Wextra -Weffc++ -Wconversion
          -pedantic -Werror=return-type -Wno-unused-command-line-argument
          -Wno-gnu-zero-variadic-macro-arguments -Wno-float-conversion
          -Wno-nested-anon-types -Wno-gnu-anonymous-struct -Wno-unused-parameter
          -Wno-gnu-conditional-omitted-operand -Wno-shorten-64-to-32
          -Wno-implicit-int-conversion -Wno-sign-conversion -Wno-deprecated-declarations
          -Wno-nonportable-include-path)
endif ()

set(SKIA_DIR third_party/skia)
get_filename_component(SKIA_DIR "${SKIA_DIR}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

list(APPEND BENCH_INCLUDES ${SKIA_DIR})
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

list(APPEND BENCH_INCLUDES src)
file(GLOB_RECURSE SRC_FILES src/*.cpp)

if (IOS)
  file(GLOB_RECURSE PLATFORM_FILES src/platform/ios/*.mm)
  list(APPEND SRC_FILES ${PLATFORM_FILES})
elseif (ANDROID)
  file(GLOB_RECURSE PLATFORM_FILES src/platform/android/*.cpp)
  list(APPEND SRC_FILES ${PLATFORM_FILES})
elseif (MACOS)
  file(GLOB_RECURSE PLATFORM_FILES src/platform/mac/*.mm)
  list(APPEND SRC_FILES ${PLATFORM_FILES})
  find_library(COCOA Cocoa)
  find_library(OPENGL OpenGL)
  find_library(COREVIDEO CoreVideo)
  list(APPEND BENCHMARK_LIBS ${COCOA} ${OPENGL} ${COREVIDEO})
elseif (WIN32)
  file(GLOB_RECURSE PLATFORM_FILES src/platform/win/*.cpp)
  list(APPEND SRC_FILES ${PLATFORM_FILES})
endif ()
list(APPEND BENCHMARK_LIBS ${SKIA_SHARED_LIBS})

list(APPEND BENCH_STATIC_VENDORS skia)
add_vendor_target(bench-vendor STATIC_VENDORS ${BENCH_STATIC_VENDORS})
find_vendor_libraries(bench-vendor STATIC BENCH_VENDOR_STATIC_LIBRARIES)

add_executable(${COMP_TARGET} ${SRC_FILES})
add_dependencies(${COMP_TARGET} bench-vendor)

target_compile_definitions(${COMP_TARGET} PRIVATE
        -DSK_GANESH
        -DSK_GL
        -DSK_DEFAULT_TYPEFACE_IS_EMPTY
        -DSK_DISABLE_LEGACY_DEFAULT_TYPEFACE
        -DSK_ENABLE_PRECOMPILE
        -DSK_ASSUME_GL=1
        -DSK_USE_PERFETTO
        -DSK_ENABLE_API_AVAILABLE
        -DSK_GAMMA_APPLY_TO_A8
        -DSKIA_IMPLEMENTATION=1
)

target_include_directories(${COMP_TARGET} PRIVATE ${BENCH_INCLUDES})
target_compile_options(${COMP_TARGET} PUBLIC ${BENCH_COMPILE_OPTIONS})
target_link_libraries(${COMP_TARGET} PUBLIC ${BENCH_VENDOR_STATIC_LIBRARIES} ${BENCHMARK_LIBS})


