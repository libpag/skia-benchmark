/////////////////////////////////////////////////////////////////////////////////////////////////
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

#include <emscripten/bind.h>
#include <emscripten/html5.h>
#include "base/AppHost.h"
#include "benchmark/ParticleBench.h"
#include "include/gpu/ganesh/GrDirectContext.h"

using namespace emscripten;
namespace benchmark {
class SkiaView {
 public:
  SkiaView(const std::string& canvasID);

  ~SkiaView();

  void setImagePath(const std::string& imagePath);

  void registerFonts(const val& fontVal, const val& emojiFontVal);

  void updateSize(float devicePixelRatio);

  void startDraw();

  void draw();

  void restartDraw() const;

  void updatePerfInfo(const PerfData& data) const;

  void updateDrawParam(const DrawParam& drawParam) const;

  void updateGraphicType(int type);

  ParticleBench* getBenchByIndex() const;

  void showPerfData(bool status);

  void setAntiAlias(bool aa);

  void setStroke(bool stroke);

  void setLineJoinType(int type);

  int drawIndex = 0;
  std::shared_ptr<benchmark::AppHost> appHost = nullptr;
  bool showPerfDataFlag = true;

 private:
  std::string canvasID = "";
  sk_sp<GrDirectContext> skContext = nullptr;
  sk_sp<SkSurface> skSurface = nullptr;
};
}  // namespace benchmark
