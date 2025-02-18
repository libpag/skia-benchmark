/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLWindowContext.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "src/base/SkMathPriv.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"

namespace skiawindow {

GLWindowContext::GLWindowContext(const DisplayParams& params)
    : WindowContext(params), backendContext(nullptr), surface(nullptr) {
  _displayParams.MSAASampleCount = GrNextPow2(_displayParams.MSAASampleCount);
}

void GLWindowContext::initializeContext() {
  SkASSERT(!context);

  auto interface = this->onInitializeContext();
  if (!interface) {
    return;
  }
  backendContext = interface;

  context = GrDirectContexts::MakeGL(backendContext, _displayParams.grContextOptions);
  if (!context && _displayParams.MSAASampleCount > 1) {
    _displayParams.MSAASampleCount /= 2;
    this->initializeContext();
  }
}

void GLWindowContext::destroyContext() {
  surface.reset(nullptr);

  if (context) {
    // in case we have outstanding refs to this (lua?)
    context->abandonContext();
    context.reset();
  }

  backendContext.reset(nullptr);

  this->onDestroyContext();
}

sk_sp<SkSurface> GLWindowContext::getBackbufferSurface() {
  if (nullptr == surface) {
    if (context) {
      GrGLFramebufferInfo fbInfo;
      fbInfo.fFBOID = 0;
      fbInfo.fFormat = GR_GL_RGBA8;
      fbInfo.fProtected = skgpu::Protected(_displayParams.createProtectedNativeBackend);

      auto backendRT =
          GrBackendRenderTargets::MakeGL(_width, _height, _sampleCount, _stencilBits, fbInfo);

      surface = SkSurfaces::WrapBackendRenderTarget(
          context.get(), backendRT, kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
          _displayParams.colorSpace, &_displayParams.surfaceProps);
    }
  }

  return surface;
}

void GLWindowContext::resize(int w, int h) {
  this->destroyContext();
  this->initializeContext();
}

void GLWindowContext::setDisplayParams(const DisplayParams& params) {
  _displayParams = params;
  this->destroyContext();
  this->initializeContext();
}

}  // namespace skiawindow
