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
#pragma once

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "window_context/DisplayParams.h"

class SkSurface;

namespace skiawindow {

class WindowContext {
 public:
  WindowContext(const DisplayParams&);

  virtual ~WindowContext();

  virtual sk_sp<SkSurface> getBackbufferSurface() = 0;

  void swapBuffers();

  virtual bool isValid() = 0;

  virtual void resize(int w, int h) = 0;

  virtual void activate(bool isActive) {
  }

  const DisplayParams& displayParams() {
    return _displayParams;
  }
  virtual void setDisplayParams(const DisplayParams& params) = 0;

  GrDirectContext* directContext() const {
    return context.get();
  }

  int width() const {
    return _width;
  }
  int height() const {
    return _height;
  }

  int sampleCount() const {
    return _sampleCount;
  }
  int stencilBits() const {
    return _stencilBits;
  }

 protected:
  virtual void onSwapBuffers() = 0;

  sk_sp<GrDirectContext> context;

  int _width{1024};
  int _height{720};
  DisplayParams _displayParams;

  int _sampleCount = 1;
  int _stencilBits = 0;
};

}  // namespace skiawindow
