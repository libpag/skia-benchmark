@echo off

:: ROOT_PATH=~dp0/../../
set "script_path=%~dp0"
for %%i in ("%script_path%.") do set "script_path=%%~dpi"
for %%i in ("%script_path%.") do set "script_path=%%~dpi"
set "ROOT_PATH=%script_path%"

python %ROOT_PATH%\third_party\skia\tools\git-sync-deps

for %%a in (%VENDOR_ARCHS%) do  call :buildArch %%a
goto :eof

:buildArch
  setlocal enabledelayedexpansion
  set arch=%1
  set OUT_REAL_PATH=out\win\%arch%\
  call :make_dir %OUT_REAL_PATH%
  set GN_ARGS=
  if "%VENDOR_BUILD_TYPE%"=="Debug" (
    set GN_ARGS=target_cpu=\"%arch%\" ^
                is_debug=true ^
                extra_cflags=[\"/MDd\"]
  ) else (
    set GN_ARGS=target_cpu=\"%arch%\" ^
                is_official_build=true ^
                is_debug=false ^
                skia_enable_pdf=false ^
                skia_enable_skottie=false ^
                skia_enable_skparagraph=false ^
                skia_enable_skshaper=false ^
                skia_enable_skshaper_tests=false ^
                skia_enable_svg=false ^
                skia_enable_tools=false ^
                skia_use_system_libjpeg_turbo=false ^
                skia_use_system_libpng=false ^
                skia_use_system_libwebp=false ^
                skia_use_system_expat=false ^
                skia_use_system_zlib=false ^
                extra_cflags=[\"/MD\"]
  )

  bin\gn gen %OUT_REAL_PATH% --args="!GN_ARGS!"
  ninja -C %OUT_REAL_PATH%

  set SOURCE_DIR=%CD%
  set BUILD_DIR=%SOURCE_DIR%\%OUT_REAL_PATH%
  set LIBRARY_OUT_DIR=%VENDOR_OUT_DIR%\win\%arch%

  mkdir %LIBRARY_OUT_DIR%
  copy %BUILD_DIR%\*.lib %LIBRARY_OUT_DIR%

  rd /s /q %BUILD_DIR%

  endlocal
goto :eof

:make_dir
  if exist "%1" (
      rd /s /q "%1"
  )
  mkdir "%1"
goto :eof