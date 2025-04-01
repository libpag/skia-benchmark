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

#include <emscripten/bind.h>
#include "SkiaView.h"
#include "benchmark/ParticleBench.h"
using namespace benchmark;
using namespace emscripten;

EMSCRIPTEN_BINDINGS(SkiaDemo) {

  class_<SkiaView>("SkiaView")
      .smart_ptr<std::shared_ptr<SkiaView>>("SkiaView")
      .class_function("MakeFrom", optional_override([](const std::string& canvasID) {
                        if (canvasID.empty()) {
                          return std::shared_ptr<SkiaView>(nullptr);
                        }
                        return std::make_shared<SkiaView>(canvasID);
                      }))
      .function("setImagePath", &SkiaView::setImagePath)
      .function("updateSize", &SkiaView::updateSize)
      .function("startDraw", &SkiaView::startDraw)
      .function("registerFonts", &SkiaView::registerFonts)
      .function("restartDraw", &SkiaView::restartDraw)
      .function("updateDrawParam", &SkiaView::updateDrawParam)
      .function("updateGraphicType", &SkiaView::updateGraphicType)
      .function("showPerfData", &SkiaView::showPerfData);

  value_object<DrawParam>("DrawParam")
      .field("startCount", &DrawParam::startCount)
      .field("stepCount", &DrawParam::stepCount)
      .field("minFPS", &DrawParam::minFPS)
      .field("maxCount", &DrawParam::maxCount);
}