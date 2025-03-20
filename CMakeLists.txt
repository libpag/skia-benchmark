cmake_minimum_required(VERSION 3.13)
project(Skia-Benchmark)

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
          -Wno-nonportable-include-path -Wno-implicit-int-float-conversion)
endif ()

if (MSVC)
  add_compile_options(/wd4244)
  add_compile_options("/utf-8")
  string(TOLOWER ${MSVC_C_ARCHITECTURE_ID} ARCH)
endif (MSVC)

set(SKIA_DIR third_party/skia)
get_filename_component(SKIA_DIR "${SKIA_DIR}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

list(APPEND BENCH_INCLUDES ${SKIA_DIR})
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

list(APPEND BENCH_INCLUDES src)
file(GLOB_RECURSE SRC_FILES
        src/tools/*.cpp
        src/base/*.cpp
        src/benchmark/*.cpp
        src/window_context/*.cpp)

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
  find_package(OpenGL REQUIRED)
  list(APPEND BENCHMARK_LIBS ${OPENGL_LIBRARIES})

  if (${CMAKE_SYSTEM_VERSION} VERSION_GREATER_EQUAL 6.3) # Windows 8.1 or later
    add_definitions(-D_WIN32_WINNT=0x0603)
    find_library(SHCORE_LIB SHCore)
    list(APPEND BENCHMARK_LIBS ${SHCORE_LIB})
  else ()
    add_definitions(-D_WIN32_WINNT=0x0601) # Windows 7
    find_library(USER32 user32)
    list(APPEND BENCHMARK_LIBS ${USER32})
  endif ()
endif ()
list(APPEND BENCHMARK_LIBS ${SKIA_SHARED_LIBS})

list(APPEND BENCH_STATIC_VENDORS skia)
add_vendor_target(bench-vendor STATIC_VENDORS ${BENCH_STATIC_VENDORS})
find_vendor_libraries(bench-vendor STATIC BENCH_VENDOR_STATIC_LIBRARIES)

if (WIN32)
  add_executable(Benchmark WIN32 ${SRC_FILES})
else ()
  add_executable(Benchmark ${SRC_FILES})
endif ()
add_dependencies(Benchmark bench-vendor)

list(APPEND BENCH_DEFINES -DSK_GANESH -DSK_GL -DSK_DEFAULT_TYPEFACE_IS_EMPTY
        -DSK_DISABLE_LEGACY_DEFAULT_TYPEFACE -DSK_ENABLE_PRECOMPILE
        -DSK_GAMMA_APPLY_TO_A8 -DSKIA_IMPLEMENTATION=1)
if (WIN32)
  list(APPEND BENCH_DEFINES
          -DWIN32_LEAN_AND_MEAN
          -DNOMINMAX
          -DSK_FONTMGR_DIRECTWRITE_AVAILABLE
          -DSK_TYPEFACE_FACTORY_DIRECTWRITE
          $<$<CONFIG:Debug>:GR_TEST_UTILS=1>
          $<$<CONFIG:Debug>:SK_ENABLE_DUMP_GPU>
          $<$<CONFIG:Debug>:SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1>
          $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
  )
else ()
  list(APPEND BENCH_DEFINES -DSK_ASSUME_GL=1 -DSK_USE_PERFETTO -DSK_ENABLE_API_AVAILABLE)
endif ()

target_include_directories(Benchmark PRIVATE ${BENCH_INCLUDES})
target_compile_definitions(Benchmark PRIVATE ${BENCH_DEFINES})
target_compile_options(Benchmark PUBLIC ${BENCH_COMPILE_OPTIONS})
target_link_libraries(Benchmark PUBLIC ${BENCH_VENDOR_STATIC_LIBRARIES} ${BENCHMARK_LIBS})