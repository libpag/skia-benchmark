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

#include "include/gpu/gl/GrGLInterface.h"
#include "src/utils/win/SkWGL.h"
#include "window_context/GLWindowContext.h"
#include "platform/win/GLWindowContext_win.h"

#include <Windows.h>
#include <GL/gl.h>

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
  HWND              hwnd;
  HGLRC             hRc;
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
  hRc = SkCreateWGLContext(dc, _displayParams.MSAASampleCount, false /* deepColor */,
                                kGLPreferCompatibilityProfile_SkWGLContextRequest);
  if (nullptr == hRc) {
      return nullptr;
  }

  // Look to see if RenderDoc is attached. If so, re-create the context with a core profile
  if (wglMakeCurrent(dc, hRc)) {
      auto interface = GrGLMakeNativeInterface();
      bool renderDocAttached = interface->hasExtension("GL_EXT_debug_tool");
      interface.reset(nullptr);
      if (renderDocAttached) {
          wglDeleteContext(hRc);
          hRc = SkCreateWGLContext(dc, _displayParams.MSAASampleCount, false /* deepColor */,
                                      kGLPreferCoreProfile_SkWGLContextRequest);
          if (nullptr == hRc) {
              return nullptr;
          }
      }
  }
  SkWGLExtensions extensions;
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
    if (extensions.hasExtension(dc, "WGL_ARB_multisample")) {
      static const int kSampleCountAttr = SK_WGL_SAMPLES;
      extensions.getPixelFormatAttribiv(dc,
                                        pixelFormat,
                                        0,
                                        1,
                                        &kSampleCountAttr,
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

  if (extensions.hasExtension(dc, "WGL_EXT_swap_control")) {
    extensions.swapInterval(_displayParams.disableVsync ? 0 : 1);
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
