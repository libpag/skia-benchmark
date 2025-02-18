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

#include "SkiaWindow.h"
#include <include/core/SkFontMgr.h>
#include "GLWindowContext_win.h"
#include "include/core/SkSurface.h"
#if WINVER >= 0x0603  // Windows 8.1
#include <shellscalingapi.h>
#endif

namespace benchmark {
static constexpr LPCWSTR ClassName = L"SkiaWindow";

SkiaWindow::SkiaWindow() {
  createAppHost();
}

SkiaWindow::~SkiaWindow() {
  destroy();
}

bool SkiaWindow::open() {
  destroy();
  WNDCLASS windowClass = RegisterWindowClass();
  auto pixelRatio = getPixelRatio();
  int initWidth = static_cast<int>(pixelRatio * 1024);
  int initHeight = static_cast<int>(pixelRatio * 720);
  windowHandle = CreateWindowEx(WS_EX_APPWINDOW, windowClass.lpszClassName, L"Skia Benchmark",
                                WS_OVERLAPPEDWINDOW, 0, 0, initWidth, initHeight, nullptr, nullptr,
                                windowClass.hInstance, this);

  if (windowHandle == nullptr) {
    return false;
  }
  SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  centerAndShow();
  return true;
}

WNDCLASS SkiaWindow::RegisterWindowClass() {
  auto hInstance = GetModuleHandle(nullptr);
  WNDCLASS windowClass{};
  windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
  windowClass.lpszClassName = ClassName;
  windowClass.style = CS_HREDRAW | CS_VREDRAW;
  windowClass.cbClsExtra = 0;
  windowClass.cbWndExtra = 0;
  windowClass.hInstance = hInstance;
  windowClass.hIcon = nullptr;
  windowClass.hbrBackground = nullptr;
  windowClass.lpszMenuName = nullptr;
  windowClass.lpfnWndProc = WndProc;
  RegisterClass(&windowClass);
  return windowClass;
}

LRESULT CALLBACK SkiaWindow::WndProc(HWND window, UINT message, WPARAM wparam,
                                     LPARAM lparam) noexcept {
  auto skiaWindow = reinterpret_cast<SkiaWindow*>(GetWindowLongPtr(window, GWLP_USERDATA));
  if (skiaWindow != nullptr) {
    return skiaWindow->handleMessage(window, message, wparam, lparam);
  }
  return DefWindowProc(window, message, wparam, lparam);
}

LRESULT SkiaWindow::handleMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) noexcept {
  LRESULT result = 0;
  switch (message) {
    case WM_DESTROY:
      destroy();
      PostQuitMessage(0);
      break;
    case WM_PAINT: {
      // Commented out the next 3 lines to trigger continuous redraw
      //      PAINTSTRUCT ps;
      //      BeginPaint(windowHandle, &ps);
      draw();
      //      EndPaint(windowHandle, &ps);
      break;
    }
    case WM_LBUTTONDOWN:
      lastDrawIndex++;
      ::InvalidateRect(windowHandle, nullptr, TRUE);
      break;
    default:
      result = DefWindowProc(windowHandle, message, wparam, lparam);
      break;
  }
  return result;
}

void SkiaWindow::destroy() {
  if (windowHandle) {
    DestroyWindow(windowHandle);
    windowHandle = nullptr;
    UnregisterClass(ClassName, nullptr);
  }
}

void SkiaWindow::centerAndShow() const {
  if ((GetWindowStyle(windowHandle) & WS_CHILD) != 0) {
    return;
  }
  RECT rcDlg = {0};
  ::GetWindowRect(windowHandle, &rcDlg);
  RECT rcArea = {0};
  RECT rcCenter = {0};
  HWND hWnd = windowHandle;
  HWND hWndCenter = ::GetWindowOwner(windowHandle);
  if (hWndCenter != nullptr) {
    hWnd = hWndCenter;
  }

  MONITORINFO oMonitor = {};
  oMonitor.cbSize = sizeof(oMonitor);
  ::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
  rcArea = oMonitor.rcWork;

  if (hWndCenter == nullptr) {
    rcCenter = rcArea;
  } else {
    ::GetWindowRect(hWndCenter, &rcCenter);
  }

  int DlgWidth = rcDlg.right - rcDlg.left;
  int DlgHeight = rcDlg.bottom - rcDlg.top;

  // Find dialog's upper left based on rcCenter
  int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
  int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

  // The dialog is outside the screen, move it inside
  if (xLeft < rcArea.left) {
    if (xLeft < 0) {
      xLeft = GetSystemMetrics(SM_CXSCREEN) / 2 - DlgWidth / 2;
    } else {
      xLeft = rcArea.left;
    }
  } else if (xLeft + DlgWidth > rcArea.right) {
    xLeft = rcArea.right - DlgWidth;
  }

  if (yTop < rcArea.top) {
    if (yTop < 0) {
      yTop = GetSystemMetrics(SM_CYSCREEN) / 2 - DlgHeight / 2;
    } else {
      yTop = rcArea.top;
    }

  } else if (yTop + DlgHeight > rcArea.bottom) {
    yTop = rcArea.bottom - DlgHeight;
  }
  ::SetWindowPos(windowHandle, nullptr, xLeft, yTop, -1, -1,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

float SkiaWindow::getPixelRatio() const {
#if WINVER >= 0x0603  // Windows 8.1
  HMONITOR monitor = nullptr;
  if (windowHandle != nullptr) {
    monitor = ::MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
  } else {
    monitor = ::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY);
  }
  UINT dpiX = 96;
  UINT dpiY = 96;
  GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
  return static_cast<float>(dpiX) / 96.0f;
#else
  return 1.0f;
#endif
}

void SkiaWindow::createAppHost() {
  appHost = std::make_unique<AppHost>();
  auto typeface = SkFontMgr::RefDefault()->matchFamilyStyle("Microsoft Yahei", SkFontStyle());
  appHost->addTypeface("default", typeface);
}

void SkiaWindow::draw() {
  if (!windowContext) {
    windowContext = skiawindow::MakeGLForWin(windowHandle, skiawindow::DisplayParams());
  }
  if (windowContext == nullptr) {
    return;
  }
  RECT rect;
  GetClientRect(windowHandle, &rect);
  const auto width = static_cast<int>(rect.right - rect.left);
  const auto height = static_cast<int>(rect.bottom - rect.top);
  auto pixelRatio = getPixelRatio();
  auto sizeChanged = appHost->updateScreen(width, height, pixelRatio);
  if (sizeChanged) {
    windowContext->resize(width, height);
  }

  auto surface = windowContext->getBackbufferSurface();
  if (surface == nullptr) {
    return;
  }
  auto canvas = surface->getCanvas();
  canvas->clear(SK_ColorWHITE);
  auto numBenches = benchmark::Bench::Count();
  auto index = (lastDrawIndex % numBenches);
  auto bench = benchmark::Bench::GetByIndex(index);
  bench->draw(canvas, appHost.get());
  if (const auto dContext = windowContext->directContext()) {
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
  }
  windowContext->swapBuffers();
}
}  // namespace benchmark