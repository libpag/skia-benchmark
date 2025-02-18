/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making skia-benchmark available.
//
//  Copyright (C) 2025 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
//  in compliance with the License. You may obtain a copy of the License at
//
//      https://opensource.org/licenses/BSD-3-Clause
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include "SkiaWindow.h"
#if WINVER >= 0x0603  // Windows 8.1
#include <shellscalingapi.h>
#endif

extern "C" {
//Enable dedicated graphics for NVIDIA:
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
//Enable dedicated graphics for AMD:
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
#if WINVER >= 0x0603  // Windows 8.1
  SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#else
  SetProcessDPIAware();
#endif

  benchmark::SkiaWindow skiaWindow = {};
  skiaWindow.open();

  MSG msg = {};
  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    if (msg.message == WM_QUIT) {
      return (int)msg.wParam;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}