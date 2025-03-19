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
#pragma once

#include "WindowContext.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"

namespace skiawindow {

class GLWindowContext : public WindowContext {
 public:
  sk_sp<SkSurface> getBackbufferSurface() override;

  bool isValid() override {
    return SkToBool(backendContext.get());
  }

  void resize(int w, int h) override;

  void setDisplayParams(const DisplayParams& params) override;

 protected:
  explicit GLWindowContext(const DisplayParams&);
  // This should be called by subclass constructor. It is also called when window/display
  // parameters change. This will in turn call onInitializeContext().
  void initializeContext();
  virtual sk_sp<const GrGLInterface> onInitializeContext() = 0;

  // This should be called by subclass destructor. It is also called when window/display
  // parameters change prior to initializing a new GL context. This will in turn call
  // onDestroyContext().
  void destroyContext();
  virtual void onDestroyContext() = 0;

  sk_sp<const GrGLInterface> backendContext;
  sk_sp<SkSurface> surface;
};

}  // namespace skiawindow
