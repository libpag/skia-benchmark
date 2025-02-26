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

#pragma once
#include <deque>
#include "../base/Bench.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"

namespace benchmark {

struct RectData {
  SkRect rect{0, 0, 1, 1};
  float speedX;
  float speedY;
};

class ParticleBench : public Bench {
 public:
  ParticleBench() : Bench("ParticleBench") {
  }

 protected:
  void onDraw(SkCanvas* canvas, const AppHost* host) override;

 private:
  void Init(const AppHost* host);

  void AnimateRects(const AppHost* host);

  void DrawRects(SkCanvas* canvas) const;

  void DrawStatus(SkCanvas* canvas, const AppHost* host);

 private:
  float width = 0;   //appHost width
  float height = 0;  //appHost height
  float targetFPS = 60.0f;
  float currentFPS = 0.f;
  size_t drawCount = 1;
  bool maxDrawCountReached = false;
  std::vector<RectData> rects = {};
  SkRect startRect = SkRect::MakeEmpty();
  SkPaint paints[3];  // red, green, blue solid paints
  int64_t lastFlushTime = -1;
  SkFont fpsFont = {};
  SkColor4f fpsColor = SkColors::kGreen;
  std::vector<std::string> status = {};
};

}  // namespace benchmark
