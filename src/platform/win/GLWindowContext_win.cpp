//
//  Tencent is pleased to support the open source community by making skia-benchmark available.
//
//  Copyright (C) 2025 Tencent. All rights reserved.
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

#include "platform/win/GLWindowContext_win.h"
#include <GL/gl.h>
#include <Windows.h>
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "platform/win/WGLInterface.h"
#include "platform/win/WGLUtil.h"
#include "window_context/GLWindowContext.h"

using skiawindow::DisplayParams;
using skiawindow::GLWindowContext;

namespace {
class GLWindowContext_win final : public GLWindowContext {
 public:
  GLWindowContext_win(HWND, const DisplayParams&);
  ~GLWindowContext_win() override;

 protected:
  void onSwapBuffers() override;

  sk_sp<const GrGLInterface> onInitializeContext() override;
  void onDestroyContext() override;

 private:
  HWND hwnd;
  HGLRC hRc;
};

GLWindowContext_win::GLWindowContext_win(HWND wnd, const DisplayParams& params)
    : GLWindowContext(params), hwnd(wnd), hRc(nullptr) {
  this->initializeContext();
}

GLWindowContext_win::~GLWindowContext_win() {
  this->destroyContext();
}

sk_sp<const GrGLInterface> GLWindowContext_win::onInitializeContext() {
  HDC dc = GetDC(hwnd);
  hRc = skiawindow::CreateWGLContext(dc, _displayParams.MSAASampleCount, nullptr, false);
  if (nullptr == hRc) {
    return nullptr;
  }

  auto wglInterface = skiawindow::WGLInterface::Get();
  // Look to see if RenderDoc is attached. If so, re-create the context with a core profile
  if (wglMakeCurrent(dc, hRc)) {
    if (wglInterface->debugToolActive) {
      wglDeleteContext(hRc);
      hRc = skiawindow::CreateWGLContext(dc, _displayParams.MSAASampleCount, nullptr, true);
      if (nullptr == hRc) {
        return nullptr;
      }
    }
  }

  if (wglMakeCurrent(dc, hRc)) {
    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // use DescribePixelFormat to get the stencil and color bit depth.
    int pixelFormat = GetPixelFormat(dc);
    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(dc, pixelFormat, sizeof(pfd), &pfd);
    _stencilBits = pfd.cStencilBits;

    // Get sample count if the MSAA WGL extension is present
    if (wglInterface->multiSampleSupport) {
      static const int kSampleCountAttr = WGL_SAMPLES;
      wglInterface->wglGetPixelFormatAttribiv(dc, pixelFormat, 0, 1, &kSampleCountAttr,
                                              &_sampleCount);
      _sampleCount = std::max(_sampleCount, 1);
    } else {
      _sampleCount = 1;
    }

    RECT rect;
    GetClientRect(hwnd, &rect);
    _width = rect.right - rect.left;
    _height = rect.bottom - rect.top;
    glViewport(0, 0, _width, _height);
  }

  if (wglInterface->swapIntervalSupport) {
    wglInterface->wglSwapInterval(_displayParams.disableVsync ? 0 : 1);
  }

  return GrGLMakeNativeInterface();
}

void GLWindowContext_win::onDestroyContext() {
  wglDeleteContext(hRc);
  hRc = nullptr;
}

void GLWindowContext_win::onSwapBuffers() {
  HDC dc = GetDC((HWND)hwnd);
  SwapBuffers(dc);
  ReleaseDC((HWND)hwnd, dc);
}

}  // anonymous namespace

namespace skiawindow {

std::unique_ptr<WindowContext> MakeGLForWin(HWND wnd, const DisplayParams& params) {
  std::unique_ptr<WindowContext> ctx(new GLWindowContext_win(wnd, params));
  if (!ctx->isValid()) {
    return nullptr;
  }
  return ctx;
}

}  // namespace skiawindow
