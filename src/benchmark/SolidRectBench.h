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
  float speed;
};

class SolidRectBench : public Bench {
 public:
  SolidRectBench() : Bench("SolidRectBench") {
  }

 protected:
  void onDraw(SkCanvas* canvas, const AppHost* host) override;

 private:
  void Init(const AppHost* host);

  void InitRects();

  void InitPaints();

  void DrawRects(SkCanvas* canvas) const;

  void DrawFPS(SkCanvas* canvas, const AppHost* host);

  void AnimateRects();

 private:
  float width = 0;   //appHost width
  float height = 0;  //appHost height
  size_t frameCount = 0;
  size_t curRectCount = 0;
  std::string fpsInfo;
  std::vector<RectData> rects;
  SkPaint paints[3];           // red, green, blue solid paints
  SkPaint fpsBackgroundpaint;  // red solid paint
  SkPaint fpsTextPaint;        // white solid paint
  uint64_t lastMs = 0;
  std::deque<uint64_t> timeStamps;
  SkRect fpsBackgroundRect;
  SkFont fpsFont;
};

}  // namespace benchmark
