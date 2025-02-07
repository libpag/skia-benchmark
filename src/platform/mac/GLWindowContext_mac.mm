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

#include "platform/mac/GLWindowContext_mac.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "window_context/GLWindowContext.h"

#include <Cocoa/Cocoa.h>
#include <OpenGL/gl.h>

using skiawindow::DisplayParams;
using skiawindow::GLWindowContext;
using skiawindow::MacWindowInfo;

namespace {

class GLWindowContext_mac : public GLWindowContext {
 public:
  GLWindowContext_mac(const MacWindowInfo&, const DisplayParams&);

  ~GLWindowContext_mac() override;

  sk_sp<const GrGLInterface> onInitializeContext() override;
  void onDestroyContext() override;

  void resize(int w, int h) override;

 private:
  void teardownContext();
  void onSwapBuffers() override;

  NSView* mainView;
  NSOpenGLContext* GLContext;
  NSOpenGLPixelFormat* pixelFormat;
};

GLWindowContext_mac::GLWindowContext_mac(const MacWindowInfo& info, const DisplayParams& params)
    : GLWindowContext(params), mainView(info.mainView), GLContext(nil) {
  this->initializeContext();
}

GLWindowContext_mac::~GLWindowContext_mac() {
  teardownContext();
}

void GLWindowContext_mac::teardownContext() {
  [NSOpenGLContext clearCurrentContext];
  [pixelFormat release];
  pixelFormat = nil;
  [GLContext release];
  GLContext = nil;
}

sk_sp<const GrGLInterface> GLWindowContext_mac::onInitializeContext() {
  SkASSERT(nil != mainView);

  if (!GLContext) {
    pixelFormat = skiawindow::GetGLPixelFormat(_displayParams.MSAASampleCount);
    if (nil == pixelFormat) {
      return nullptr;
    }

    // create context
    GLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    if (nil == GLContext) {
      [pixelFormat release];
      pixelFormat = nil;
      return nullptr;
    }

    [mainView setWantsBestResolutionOpenGLSurface:YES];
    [GLContext setView:mainView];
  }

  GLint swapInterval = _displayParams.disableVsync ? 0 : 1;
  [GLContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

  // make context current
  [GLContext makeCurrentContext];

  glClearStencil(0);
  glClearColor(0, 0, 0, 255);
  glStencilMask(0xffffffff);
  glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  GLint stencilBits;
  [pixelFormat getValues:&stencilBits forAttribute:NSOpenGLPFAStencilSize forVirtualScreen:0];
  _stencilBits = stencilBits;
  GLint sampleCount;
  [pixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];
  _sampleCount = sampleCount;
  _sampleCount = std::max(_sampleCount, 1);

  CGFloat backingScaleFactor = skiawindow::GetBackingScaleFactor(mainView);
  _width = mainView.bounds.size.width * backingScaleFactor;
  _height = mainView.bounds.size.height * backingScaleFactor;
  glViewport(0, 0, _width, _height);

  auto interface = GrGLMakeNativeInterface();
  return interface;
}

void GLWindowContext_mac::onDestroyContext() {
  // We only need to tear down the GLContext if we've changed the sample count.
  if (GLContext && _sampleCount != _displayParams.MSAASampleCount) {
    teardownContext();
  }
}

void GLWindowContext_mac::onSwapBuffers() {
  [GLContext flushBuffer];
}

void GLWindowContext_mac::resize(int w, int h) {
  [GLContext update];

  // The super class always recreates the context.
  GLWindowContext::resize(0, 0);
}

}  // anonymous namespace

namespace skiawindow {

std::unique_ptr<WindowContext> MakeGLForMac(const MacWindowInfo& info,
                                            const DisplayParams& params) {
  std::unique_ptr<WindowContext> ctx(new GLWindowContext_mac(info, params));
  if (!ctx->isValid()) {
    return nullptr;
  }
  return ctx;
}

}  // namespace skiawindow
